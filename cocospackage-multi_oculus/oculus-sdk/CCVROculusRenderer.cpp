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
#include "CCVROculusRenderer.h"
#include "CCVROculusHeadTracker.h"
#include "renderer/CCRenderer.h"
#include "renderer/CCGLProgramState.h"
#include "renderer/ccGLStateCache.h"
#include "base/CCDirector.h"
#include "2d/CCScene.h"
#include "2d/CCCamera.h"
#include "2d/CCSprite.h"
#include "platform/CCGLView.h"

struct DepthBuffer
{
    GLuint        texId;

    DepthBuffer(OVR::Sizei size, int sampleCount)
    {
        GP_ASSERT(sampleCount <= 1); // The code doesn't currently handle MSAA textures.

        glGenTextures(1, &texId);
        glBindTexture(GL_TEXTURE_2D, texId);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        GLenum internalFormat = GL_DEPTH_COMPONENT24;
        GLenum type = GL_UNSIGNED_INT;
        if (true/*GLE_ARB_depth_buffer_float*/)
        {
            internalFormat = GL_DEPTH_COMPONENT32F;
            type = GL_FLOAT;
        }

        glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, size.w, size.h, 0, GL_DEPTH_COMPONENT, type, NULL);
    }
    ~DepthBuffer()
    {
        if (texId)
        {
            glDeleteTextures(1, &texId);
            texId = 0;
        }
    }
};
//--------------------------------------------------------------------------
struct TextureBuffer
{
    ovrHmd              hmd;
    ovrSwapTextureSet*  TextureSet;
    GLuint              texId;
    GLuint              fboId;
    OVR::Sizei          texSize;

    TextureBuffer(ovrHmd hmd, bool rendertarget, bool displayableOnHmd, OVR::Sizei size, int mipLevels, unsigned char * data, int sampleCount) :
        hmd(hmd),
        TextureSet(nullptr),
        texId(0),
        fboId(0),
        texSize(0, 0)
    {
        GP_ASSERT(sampleCount <= 1); // The code doesn't currently handle MSAA textures.

        texSize = size;

        if (displayableOnHmd)
        {
            // This texture isn't necessarily going to be a rendertarget, but it usually is.
            GP_ASSERT(hmd); // No HMD? A little odd.
            GP_ASSERT(sampleCount == 1); // ovr_CreateSwapTextureSetD3D11 doesn't support MSAA.

            ovrResult result = ovr_CreateSwapTextureSetGL(hmd, GL_SRGB8_ALPHA8, size.w, size.h, &TextureSet);

            if (OVR_SUCCESS(result))
            {
                for (int i = 0; i < TextureSet->TextureCount; ++i)
                {
                    ovrGLTexture* tex = (ovrGLTexture*)&TextureSet->Textures[i];
                    glBindTexture(GL_TEXTURE_2D, tex->OGL.TexId);

                    if (rendertarget)
                    {
                        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                    }
                    else
                    {
                        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
                        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
                        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
                    }
                }
            }
        }
        else
        {
            glGenTextures(1, &texId);
            glBindTexture(GL_TEXTURE_2D, texId);

            if (rendertarget)
            {
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            }
            else
            {
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            }

            glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB8_ALPHA8, texSize.w, texSize.h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        }

        if (mipLevels > 1)
        {
            glGenerateMipmap(GL_TEXTURE_2D);
        }

        glGenFramebuffers(1, &fboId);
    }

    ~TextureBuffer()
    {
        if (TextureSet)
        {
            ovr_DestroySwapTextureSet(hmd, TextureSet);
            TextureSet = nullptr;
        }
        if (texId)
        {
            glDeleteTextures(1, &texId);
            texId = 0;
        }
        if (fboId)
        {
            glDeleteFramebuffers(1, &fboId);
            fboId = 0;
        }
    }

    OVR::Sizei GetSize() const
    {
        return texSize;
    }

    void SetAndClearRenderSurface(DepthBuffer* dbuffer)
    {
        auto tex = reinterpret_cast<ovrGLTexture*>(&TextureSet->Textures[TextureSet->CurrentIndex]);

        glBindFramebuffer(GL_FRAMEBUFFER, fboId);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex->OGL.TexId, 0);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, dbuffer->texId, 0);

        //glViewport(0, 0, texSize.w, texSize.h);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        //glEnable(GL_FRAMEBUFFER_SRGB);
    }

    void UnsetRenderSurface()
    {
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, 0, 0);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, 0, 0);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void BlitFramebuffer(int w, int h) {
        auto tex = reinterpret_cast<ovrGLTexture*>(&TextureSet->Textures[TextureSet->CurrentIndex]);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fboId);
        glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex->OGL.TexId, 0);
        //glViewport(0, 0, texSize.w, texSize.h);
        glClear(GL_COLOR_BUFFER_BIT);
        //glEnable(GL_FRAMEBUFFER_SRGB);

        glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
        glBlitFramebuffer(0, 0, w, h, 0, 0, texSize.w, texSize.h,
            GL_COLOR_BUFFER_BIT, GL_LINEAR);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    }
};

NS_CC_BEGIN

VROculusRenderer::VROculusRenderer()
{
    _headTracker = new VROculusHeadTracker;
}

VROculusRenderer::~VROculusRenderer()
{
    CC_SAFE_DELETE(_headTracker);
}

void VROculusRenderer::setup(GLView* glview)
{
    ovrResult result = ovr_Initialize(nullptr);
    if (!OVR_SUCCESS(result)) {
        CCLOG("Failed to initialize libOVR.");
        return;
    };

    ovrGraphicsLuid luid;
    result = ovr_Create(&_HMD, &luid);
    if (!OVR_SUCCESS(result)) {
        CCLOG("Failed to create HMD.");
        return;
    }
    ovrHmdDesc hmdDesc = ovr_GetHmdDesc(_HMD);

    // Make eye render buffers
    for (int eye = 0; eye < EYE_NUM; ++eye){
        ovrSizei idealTextureSize = ovr_GetFovTextureSize(_HMD, ovrEyeType(eye), hmdDesc.DefaultEyeFov[eye], 1);
        _eyeRenderTexture[eye] = new TextureBuffer(_HMD, true, true, idealTextureSize, 1, nullptr, 1);
        _eyeDepthBuffer[eye] = new DepthBuffer(_eyeRenderTexture[eye]->GetSize(), 0);
        auto projection = ovrMatrix4f_Projection(hmdDesc.DefaultEyeFov[eye], 0.1f, 5000.0f, ovrProjection_RightHanded | ovrProjection_ClipRangeOpenGL);
        _eyeProjections[eye].set((const GLfloat *)(projection.M[0]));
        _eyeProjections[eye].transpose();

        if (!_eyeRenderTexture[eye]->TextureSet){
            CCLOG("Failed to create texture.");
            return;
        }
    }

    auto vp = Camera::getDefaultViewport();
    // Create mirror texture and an FBO used to copy mirror texture to back buffer
    result = ovr_CreateMirrorTextureGL(_HMD, GL_SRGB8_ALPHA8, vp._width, vp._height, reinterpret_cast<ovrTexture**>(&_mirrorTexture));
    if (!OVR_SUCCESS(result)){
        CCLOG("Failed to create mirror texture.");
        return;
    }

    // Configure the mirror read buffer
    glGenFramebuffers(1, &_mirrorFBO);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, _mirrorFBO);
    glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _mirrorTexture->OGL.TexId, 0);
    glFramebufferRenderbuffer(GL_READ_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, 0);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);

    _eyeRenderDesc[0] = ovr_GetRenderDesc(_HMD, ovrEye_Left, hmdDesc.DefaultEyeFov[0]);
    _eyeRenderDesc[1] = ovr_GetRenderDesc(_HMD, ovrEye_Right, hmdDesc.DefaultEyeFov[1]);


    _ld.Header.Type = ovrLayerType_EyeFov;
    _ld.Header.Flags = ovrLayerFlag_TextureOriginAtBottomLeft;   // Because OpenGL.

    _headTracker->setHMD(_HMD);
}

void VROculusRenderer::cleanup()
{
    _headTracker->setHMD(nullptr);
    if (_mirrorFBO) glDeleteFramebuffers(1, &_mirrorFBO);
    if (_mirrorTexture) ovr_DestroyMirrorTexture(_HMD, reinterpret_cast<ovrTexture*>(_mirrorTexture));
    for (int eye = 0; eye < EYE_NUM; ++eye){
        delete _eyeRenderTexture[eye];
        delete _eyeDepthBuffer[eye];
    }
    ovr_Destroy(_HMD);
    ovr_Shutdown();
    //OVR::System::Dest
}

VRIHeadTracker* VROculusRenderer::getHeadTracker()
{
    return _headTracker;
}

void VROculusRenderer::render(Scene* scene, Renderer* renderer)
{
// 	double           ftiming = ovr_GetPredictedDisplayTime(_HMD, 0);
// 	// Keeping sensorSampleTime as close to ovr_GetTrackingState as possible - fed into the layer
// 	double           sensorSampleTime = ovr_GetTimeInSeconds();
// 	ovrTrackingState hmdState = ovr_GetTrackingState(_HMD, ftiming, ovrTrue);
// 	//Vec3 defaultPos = Camera::getDefaultCamera()->getPosition3D();

    ovrVector3f               ViewOffset[2] = { _eyeRenderDesc[0].HmdToEyeViewOffset, _eyeRenderDesc[1].HmdToEyeViewOffset };
    ovrPosef                  EyeRenderPose[2];
    double ftiming = ovr_GetPredictedDisplayTime(_HMD, 0);
    double sensorSampleTime = ovr_GetTimeInSeconds();
    _headTracker->applyTracking(ftiming);
    _ld.SensorSampleTime = sensorSampleTime;
    ovr_CalcEyePoses(_headTracker->getTracking().HeadPose.ThePose, ViewOffset, EyeRenderPose);

    Mat4 headView;
    Mat4::createTranslation(_headTracker->getLocalPosition(), &headView);
    headView *= _headTracker->getLocalRotation();

    Mat4 transform;
    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);

    for (unsigned short eye = 0; eye < EYE_NUM; ++eye){
        //EyeRenderPose[eye].Position = { defaultPos.x, defaultPos.y, defaultPos.z };
        _eyeRenderTexture[eye]->TextureSet->CurrentIndex = (_eyeRenderTexture[eye]->TextureSet->CurrentIndex + 1) % _eyeRenderTexture[eye]->TextureSet->TextureCount;
        _ld.ColorTexture[eye] = _eyeRenderTexture[eye]->TextureSet;
        _ld.Viewport[eye] = OVR::Recti(_eyeRenderTexture[eye]->GetSize());
        _ld.Fov[eye] = _eyeRenderDesc[eye].Fov;
        _ld.RenderPose[eye] = EyeRenderPose[eye];

        Mat4::createTranslation(ViewOffset[eye].x, ViewOffset[eye].y, ViewOffset[eye].z, &transform);
        _eyeRenderTexture[eye]->SetAndClearRenderSurface(_eyeDepthBuffer[eye]);
        Camera::setDefaultViewport(experimental::Viewport(0, 0, _ld.Viewport[eye].Size.w, _ld.Viewport[eye].Size.h));
        transform *= headView;
        scene->render(renderer, transform.getInversed(), &_eyeProjections[eye]);
        _eyeRenderTexture[eye]->UnsetRenderSurface();
    }

    // Set up positional data.
    ovrViewScaleDesc viewScaleDesc;
    viewScaleDesc.HmdSpaceToWorldScaleInMeters = 1.0f;
    viewScaleDesc.HmdToEyeViewOffset[0] = _eyeRenderDesc[0].HmdToEyeViewOffset;
    viewScaleDesc.HmdToEyeViewOffset[1] = _eyeRenderDesc[1].HmdToEyeViewOffset;
    ovrLayerHeader* layers = &_ld.Header;
    ovrResult result = ovr_SubmitFrame(_HMD, 0, &viewScaleDesc, &layers, 1);
    if (!OVR_SUCCESS(result)) {
        CCLOG("Failed to submit frame.");
    }
    // Blit mirror texture to back buffer
    glBindFramebuffer(GL_READ_FRAMEBUFFER, _mirrorFBO);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    GLint w = _mirrorTexture->OGL.Header.TextureSize.w;
    GLint h = _mirrorTexture->OGL.Header.TextureSize.h;
    glBlitFramebuffer(0, h, w, 0,
        0, 0, w, h,
        GL_COLOR_BUFFER_BIT, GL_NEAREST);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);

    glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
}

NS_CC_END