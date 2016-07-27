/****************************************************************************
 Copyright (c) 2016 Chukong Technologies Inc.

 http://www.cocos2d-x.org

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 ****************************************************************************/

#include "platform/CCPlatformMacros.h"
#include "CCVRGvrRenderer.h"
#include "CCVRGvrHeadTracker.h"
#include "renderer/CCRenderer.h"
#include "renderer/CCGLProgramState.h"
#include "renderer/ccGLStateCache.h"
#include "base/CCDirector.h"
#include "2d/CCScene.h"
#include "2d/CCCamera.h"
#include "2d/CCSprite.h"
#include "base/CCEventListenerCustom.h"
#include "base/CCEventDispatcher.h"
#include "base/CCEventType.h"
#include "platform/CCGLView.h"
#include "platform/android/jni/JniHelper.h"

static const uint64_t kPredictionTimeWithoutVsyncNanos = 50000000;
static jlong g_gvrContext = -1;

extern "C" {
    
    JNIEXPORT void JNICALL Java_org_cocos2dx_cpp_VRSDKWrapper_nativeCreate(JNIEnv*  env, jobject thiz, jlong gvrContext) {
        g_gvrContext = gvrContext;
    }
    
    JNIEXPORT void JNICALL Java_org_cocos2dx_lua_VRSDKWrapper_nativeCreate(JNIEnv*  env, jobject thiz, jlong gvrContext) {
        g_gvrContext = gvrContext;
    }
    
    JNIEXPORT void JNICALL Java_org_cocos2dx_javascript_VRSDKWrapper_nativeCreate(JNIEnv*  env, jobject thiz, jlong gvrContext) {
        g_gvrContext = gvrContext;
    }
    
}

static gvr::Mat4f PerspectiveMatrixFromView(const gvr::Rectf& fov, float z_near,
                                            float z_far) {
    gvr::Mat4f result;
    const float x_left = -std::tan(fov.left * M_PI / 180.0f) * z_near;
    const float x_right = std::tan(fov.right * M_PI / 180.0f) * z_near;
    const float y_bottom = -std::tan(fov.bottom * M_PI / 180.0f) * z_near;
    const float y_top = std::tan(fov.top * M_PI / 180.0f) * z_near;
    const float zero = 0.0f;
    
    assert(x_left < x_right && y_bottom < y_top && z_near < z_far &&
           z_near > zero && z_far > zero);
    const float X = (2 * z_near) / (x_right - x_left);
    const float Y = (2 * z_near) / (y_top - y_bottom);
    const float A = (x_right + x_left) / (x_right - x_left);
    const float B = (y_top + y_bottom) / (y_top - y_bottom);
    const float C = (z_near + z_far) / (z_near - z_far);
    const float D = (2 * z_near * z_far) / (z_near - z_far);
    
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            result.m[i][j] = 0.0f;
        }
    }
    result.m[0][0] = X;
    result.m[0][2] = A;
    result.m[1][1] = Y;
    result.m[1][2] = B;
    result.m[2][2] = C;
    result.m[2][3] = D;
    result.m[3][2] = -1;
    
    return result;
}

static gvr::Rectf ModulateRect(const gvr::Rectf& rect, float width,
                               float height) {
    gvr::Rectf result = {rect.left * width, rect.right * width,
        rect.bottom * height, rect.top * height};
    return result;
}

static gvr::Recti CalculatePixelSpaceRect(const gvr::Sizei& texture_size,
                                          const gvr::Rectf& texture_rect) {
    float width = static_cast<float>(texture_size.width);
    float height = static_cast<float>(texture_size.height);
    gvr::Rectf rect = ModulateRect(texture_rect, width, height);
    gvr::Recti result = {
        static_cast<int>(rect.left), static_cast<int>(rect.right),
        static_cast<int>(rect.bottom), static_cast<int>(rect.top)};
    return result;
}

NS_CC_BEGIN

VRGvrRenderer::VRGvrRenderer()
{
    _headTracker = new VRGvrHeadTracker;
}

VRGvrRenderer::~VRGvrRenderer()
{
    CC_SAFE_DELETE(_headTracker);
}

void VRGvrRenderer::setup(GLView* glview)
{
    _gvrApi = gvr::GvrApi::WrapNonOwned(reinterpret_cast<gvr_context *>(g_gvrContext));
    _headTracker->setGvrApi(_gvrApi.get());
    
    auto backToForegroundListener = EventListenerCustom::create(EVENT_COME_TO_FOREGROUND,
                                                                [=](EventCustom*)
                                                                {
                                                                    if (_gvrApi.get()){
                                                                        _gvrApi->RefreshViewerProfile();
                                                                        _gvrApi->ResumeTracking();
                                                                    }
                                                                }
                                                                );
    Director::getInstance()->getEventDispatcher()->addEventListenerWithFixedPriority(backToForegroundListener, -1);
    
    
    auto foregroundToBackListener = EventListenerCustom::create(EVENT_COME_TO_BACKGROUND,
                                                                [=](EventCustom*)
                                                                {
                                                                    if (_gvrApi.get()) {
                                                                        _gvrApi->PauseTracking();
                                                                    }
                                                                }
                                                                );
    Director::getInstance()->getEventDispatcher()->addEventListenerWithFixedPriority(foregroundToBackListener, -1);
}

void VRGvrRenderer::cleanup()
{
    _headTracker->setGvrApi(nullptr);
}

VRIHeadTracker* VRGvrRenderer::getHeadTracker()
{
    return _headTracker;
}

void VRGvrRenderer::render(Scene* scene, Renderer* renderer)
{
    if (!_gvrApi.get()) return;
    
    if (!_framebufferHandle.get()){
        _gvrApi->InitializeGl();
        _renderSize = _gvrApi->GetRecommendedRenderTargetSize();
        _framebufferHandle.reset(new gvr::OffscreenFramebufferHandle(_gvrApi->CreateOffscreenFramebuffer(_renderSize)));
        _renderParamsList.reset(new gvr::RenderParamsList(_gvrApi->CreateEmptyRenderParamsList()));
        _renderParamsList->SetToRecommendedRenderParams();
        for (unsigned short i = 0; i < GVR_NUM_EYES; ++i){
            auto params = _renderParamsList->GetRenderParams(i);
            auto projection = PerspectiveMatrixFromView(params.eye_fov, 0.1f, 5000.0f);
            _eyeProjections[i].set((const GLfloat *)(projection.m[0]));
            _eyeProjections[i].transpose();
        }
    }
    
    gvr::ClockTimePoint target_time = gvr::GvrApi::GetTimePointNow();
    target_time.monotonic_system_time_nanos += kPredictionTimeWithoutVsyncNanos;
    _headTracker->applyTracking(target_time);
    const auto &head_pose = _headTracker->getHeadPose();
    
    Mat4 headView = _headTracker->getLocalRotation();
    Mat4 transform;
    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);
    
    _renderParamsList->SetToRecommendedRenderParams();
    _framebufferHandle->SetActive();
    glEnable(GL_SCISSOR_TEST);
    for (unsigned short i = 0; i < GVR_NUM_EYES; ++i){
        auto params = _renderParamsList->GetRenderParams(i);
        const gvr::Recti pixel_rect = CalculatePixelSpaceRect(_renderSize, params.eye_viewport_bounds);
        experimental::Viewport vp = experimental::Viewport(pixel_rect.left, pixel_rect.bottom,
                                                           pixel_rect.right - pixel_rect.left,
                                                           pixel_rect.top - pixel_rect.bottom);
        Camera::setDefaultViewport(vp);
        glScissor(vp._left, vp._bottom, vp._width, vp._height);
        glClearColor(0.125f, 0.125f, 0.125f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        auto eyeOffset = _gvrApi->GetEyeFromHeadMatrix(i? GVR_RIGHT_EYE: GVR_LEFT_EYE);
        transform.set((const GLfloat *)(eyeOffset.m[0]));
        transform.transpose();
        transform *= headView;
        scene->render(renderer, transform.getInversed(), &_eyeProjections[i]);
    }
    glDisable(GL_SCISSOR_TEST);
    _gvrApi->SetDefaultFramebufferActive();
    _gvrApi->DistortOffscreenFramebufferToScreen(*_framebufferHandle, *_renderParamsList, &head_pose, &target_time);
    
    glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
}

NS_CC_END