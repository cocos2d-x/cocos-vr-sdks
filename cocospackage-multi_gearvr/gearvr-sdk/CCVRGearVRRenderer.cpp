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
#include "CCVRGearVRRenderer.h"
#include "CCVRGearVRHeadTracker.h"
#include "renderer/CCRenderer.h"
#include "renderer/CCGLProgramState.h"
#include "renderer/ccGLStateCache.h"
#include "base/CCDirector.h"
#include "2d/CCScene.h"
#include "2d/CCCamera.h"
#include "2d/CCSprite.h"
#include "platform/CCGLView.h"
#include "platform/android/jni/JniHelper.h"
#include "base/CCEventListenerCustom.h"
#include "base/CCEventDispatcher.h"
#include "base/CCEventType.h"

#define GL( func )		func;
#define NUM_MULTI_SAMPLES	4

static void ovrFramebuffer_Clear(ovrFramebuffer * frameBuffer)
{
    frameBuffer->Width = 0;
    frameBuffer->Height = 0;
    frameBuffer->Multisamples = 0;
    frameBuffer->TextureSwapChainLength = 0;
    frameBuffer->TextureSwapChainIndex = 0;
    frameBuffer->ColorTextureSwapChain = NULL;
    frameBuffer->DepthBuffers = NULL;
    frameBuffer->FrameBuffers = NULL;
}

static bool ovrFramebuffer_Create(ovrFramebuffer * frameBuffer, const ovrTextureFormat colorFormat, const int width, const int height, const int multisamples)
{
    frameBuffer->Width = width;
    frameBuffer->Height = height;
    frameBuffer->Multisamples = multisamples;
    
    frameBuffer->ColorTextureSwapChain = vrapi_CreateTextureSwapChain(VRAPI_TEXTURE_TYPE_2D, colorFormat, width, height, 1, true);
    frameBuffer->TextureSwapChainLength = vrapi_GetTextureSwapChainLength(frameBuffer->ColorTextureSwapChain);
    frameBuffer->DepthBuffers = (GLuint *)malloc(frameBuffer->TextureSwapChainLength * sizeof(GLuint));
    frameBuffer->FrameBuffers = (GLuint *)malloc(frameBuffer->TextureSwapChainLength * sizeof(GLuint));
    
    //PFNGLRENDERBUFFERSTORAGEMULTISAMPLEEXTPROC glRenderbufferStorageMultisampleEXT =
    //	(PFNGLRENDERBUFFERSTORAGEMULTISAMPLEEXTPROC)eglGetProcAddress("glRenderbufferStorageMultisampleEXT");
    //PFNGLFRAMEBUFFERTEXTURE2DMULTISAMPLEEXTPROC glFramebufferTexture2DMultisampleEXT =
    //	(PFNGLFRAMEBUFFERTEXTURE2DMULTISAMPLEEXTPROC)eglGetProcAddress("glFramebufferTexture2DMultisampleEXT");
    
    for (int i = 0; i < frameBuffer->TextureSwapChainLength; i++)
    {
        // Create the color buffer texture.
        const GLuint colorTexture = vrapi_GetTextureSwapChainHandle(frameBuffer->ColorTextureSwapChain, i);
        GL(glBindTexture(GL_TEXTURE_2D, colorTexture));
        GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
        GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
        GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
        GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
        GL(glBindTexture(GL_TEXTURE_2D, 0));
        
        //if (multisamples > 1 && glRenderbufferStorageMultisampleEXT != NULL && glFramebufferTexture2DMultisampleEXT != NULL)
        //{
        //	CCLOG("OVRHelper::Create multisampled buffers");
        //	// Create multisampled depth buffer.
        //	GL(glGenRenderbuffers(1, &frameBuffer->DepthBuffers[i]));
        //	GL(glBindRenderbuffer(GL_RENDERBUFFER, frameBuffer->DepthBuffers[i]));
        //	GL(glRenderbufferStorageMultisampleEXT(GL_RENDERBUFFER, multisamples, GL_DEPTH_COMPONENT24, width, height));
        //	GL(glBindRenderbuffer(GL_RENDERBUFFER, 0));
        
        //	// Create the frame buffer.
        //	GL(glGenFramebuffers(1, &frameBuffer->FrameBuffers[i]));
        //	GL(glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer->FrameBuffers[i]));
        //	GL(glFramebufferTexture2DMultisampleEXT(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTexture, 0, multisamples));
        //	GL(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, frameBuffer->DepthBuffers[i]));
        //	GL(GLenum renderFramebufferStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER));
        //	GL(glBindFramebuffer(GL_FRAMEBUFFER, 0));
        //	if (renderFramebufferStatus != GL_FRAMEBUFFER_COMPLETE)
        //	{
        //		CCLOG("OVRHelper::Incomplete frame buffer object: %s", GlFrameBufferStatusString(renderFramebufferStatus));
        //		return false;
        //	}
        //}
        //else
        {
            CCLOG("OVRHelper::Create buffers");
            // Create depth buffer.
            GL(glGenRenderbuffers(1, &frameBuffer->DepthBuffers[i]));
            GL(glBindRenderbuffer(GL_RENDERBUFFER, frameBuffer->DepthBuffers[i]));
            GL(glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24_OES, width, height));
            GL(glBindRenderbuffer(GL_RENDERBUFFER, 0));
            
            // Create the frame buffer.
            GL(glGenFramebuffers(1, &frameBuffer->FrameBuffers[i]));
            GL(glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer->FrameBuffers[i]));
            GL(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, frameBuffer->DepthBuffers[i]));
            GL(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTexture, 0));
            GL(GLenum renderFramebufferStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER));
            GL(glBindFramebuffer(GL_FRAMEBUFFER, 0));
            if (renderFramebufferStatus != GL_FRAMEBUFFER_COMPLETE)
            {
                CCLOG("OVRHelper::Incomplete frame buffer object");
                return false;
            }
        }
    }
    
    return true;
}

static void ovrFramebuffer_Destroy(ovrFramebuffer * frameBuffer)
{
    GL(glDeleteFramebuffers(frameBuffer->TextureSwapChainLength, frameBuffer->FrameBuffers));
    GL(glDeleteRenderbuffers(frameBuffer->TextureSwapChainLength, frameBuffer->DepthBuffers));
    vrapi_DestroyTextureSwapChain(frameBuffer->ColorTextureSwapChain);
    
    free(frameBuffer->DepthBuffers);
    free(frameBuffer->FrameBuffers);
    
    ovrFramebuffer_Clear(frameBuffer);
}

static void ovrFramebuffer_SetCurrent(ovrFramebuffer * frameBuffer)
{
    GL(glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer->FrameBuffers[frameBuffer->TextureSwapChainIndex]));
}

static void ovrFramebuffer_SetNone()
{
    GL(glBindFramebuffer(GL_FRAMEBUFFER, 0));
}

static void ovrFramebuffer_Resolve(ovrFramebuffer * frameBuffer)
{
    // Discard the depth buffer, so the tiler won't need to write it back out to memory.
    //const GLenum depthAttachment[1] = { GL_DEPTH_ATTACHMENT };
    //glInvalidateFramebuffer(GL_FRAMEBUFFER, 1, depthAttachment);
    
    // Flush this frame worth of commands.
    glFlush();
}

static void ovrFramebuffer_Advance(ovrFramebuffer * frameBuffer)
{
    // Advance to the next texture from the set.
    frameBuffer->TextureSwapChainIndex = (frameBuffer->TextureSwapChainIndex + 1) % frameBuffer->TextureSwapChainLength;
}

NS_CC_BEGIN

VRGearVRRenderer::VRGearVRRenderer()
    : _ovr(nullptr)
    , _frameIndex(0)
{
    _headTracker = new VRGearVRHeadTracker;
}

VRGearVRRenderer::~VRGearVRRenderer()
{
    CC_SAFE_DELETE(_headTracker);
}

void VRGearVRRenderer::setup(GLView* glview)
{
    _java.ActivityObject = JniHelper::getEnv()->NewGlobalRef(JniHelper::getActivity());
    _java.Env = JniHelper::getEnv();
    JniHelper::getEnv()->GetJavaVM(&_java.Vm);
    const ovrInitParms initParms = vrapi_DefaultInitParms(&_java);
    int32_t initResult = vrapi_Initialize(&initParms);
    
    for (int eye = 0; eye < EYE_NUM; eye++)
    {
        ovrFramebuffer_Clear(&_frameBuffer[eye]);
        ovrFramebuffer_Create(&_frameBuffer[eye],
                              VRAPI_TEXTURE_FORMAT_8888,
                              vrapi_GetSystemPropertyInt(&_java, VRAPI_SYS_PROP_SUGGESTED_EYE_TEXTURE_WIDTH),
                              vrapi_GetSystemPropertyInt(&_java, VRAPI_SYS_PROP_SUGGESTED_EYE_TEXTURE_HEIGHT),
                              NUM_MULTI_SAMPLES);
    }
    
    const float suggestedEyeFovDegreesX = vrapi_GetSystemPropertyFloat(&_java, VRAPI_SYS_PROP_SUGGESTED_EYE_FOV_DEGREES_X);
    const float suggestedEyeFovDegreesY = vrapi_GetSystemPropertyFloat(&_java, VRAPI_SYS_PROP_SUGGESTED_EYE_FOV_DEGREES_Y);
    
    _projection = ovrMatrix4f_CreateProjectionFov(suggestedEyeFovDegreesX, suggestedEyeFovDegreesY, 0.0f, 0.0f, VRAPI_ZNEAR, 5000.0f);
    _eyeProjection.set((const GLfloat *)(ovrMatrix4f_Transpose(&_projection).M[0]));
    
    auto backToForegroundListener = EventListenerCustom::create(EVENT_COME_TO_FOREGROUND,
                                                                [=](EventCustom*)
                                                                {
                                                                    if (!_ovr){
                                                                        ovrModeParms modeParms = vrapi_DefaultModeParms(&_java);
                                                                        _ovr = vrapi_EnterVrMode(&modeParms);
                                                                        _headTracker->setOVR(_ovr);
                                                                    }
                                                                }
                                                                );
    Director::getInstance()->getEventDispatcher()->addEventListenerWithFixedPriority(backToForegroundListener, -1);
    
    
    auto foregroundToBackListener = EventListenerCustom::create(EVENT_COME_TO_BACKGROUND,
                                                                [=](EventCustom*)
                                                                {
                                                                    if (_ovr){
                                                                        _headTracker->setOVR(nullptr);
                                                                        vrapi_LeaveVrMode(_ovr);
                                                                        _ovr = nullptr;
                                                                    }
                                                                }
                                                                );
    Director::getInstance()->getEventDispatcher()->addEventListenerWithFixedPriority(foregroundToBackListener, -1);

}

void VRGearVRRenderer::cleanup()
{
    for (int eye = 0; eye < EYE_NUM; eye++){
        ovrFramebuffer_Destroy(&_frameBuffer[eye]);
    }
    vrapi_Shutdown();
}

VRIHeadTracker* cocos2d::VRGearVRRenderer::getHeadTracker()
{
    return _headTracker;
}

void VRGearVRRenderer::render(Scene* scene, Renderer* renderer)
{
    if (!_ovr) return;
    ++_frameIndex;
    const ovrHeadModelParms headModelParms = vrapi_DefaultHeadModelParms();
    ovrFrameParms frameParms = vrapi_DefaultFrameParms(&_java, VRAPI_FRAME_INIT_DEFAULT, vrapi_GetTimeInSeconds(), NULL);
    frameParms.FrameIndex = _frameIndex;
    frameParms.MinimumVsyncs = 1;
    frameParms.PerformanceParms = vrapi_DefaultPerformanceParms();
    frameParms.Layers[VRAPI_FRAME_LAYER_TYPE_WORLD].Flags |= VRAPI_FRAME_LAYER_FLAG_CHROMATIC_ABERRATION_CORRECTION;
    
    const double predictedDisplayTime = vrapi_GetPredictedDisplayTime(_ovr, _frameIndex);
    _headTracker->applyTracking(predictedDisplayTime);
    
    
    Mat4 headView = _headTracker->getLocalRotation();
    Mat4 transform;
    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);
    
    glEnable(GL_SCISSOR_TEST);
    for (unsigned short i = 0; i < EYE_NUM; ++i){
        ovrFramebuffer * frameBuffer = &_frameBuffer[i];
        ovrFramebuffer_SetCurrent(frameBuffer);
        int width = frameBuffer->Width;
        int height = frameBuffer->Height;
        
        Camera::setDefaultViewport(experimental::Viewport(0, 0, width, height));
        glScissor(0, 0, width, height);
        glClearColor(0.125f, 0.125f, 0.125f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        const float eyeOffset = ( i ? -0.5f : 0.5f ) * headModelParms.InterpupillaryDistance;
        Mat4::createTranslation(eyeOffset, 0, 0, &transform);
        transform *= headView;
        scene->render(renderer, transform.getInversed(), &_eyeProjection);
        
        // Explicitly clear the border texels to black because OpenGL-ES does not support GL_CLAMP_TO_BORDER.
        {
            // Clear to fully opaque black.
            glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
            // bottom
            glScissor(0, 0, frameBuffer->Width, 1);
            glClear(GL_COLOR_BUFFER_BIT);
            // top
            glScissor(0, frameBuffer->Height - 1, frameBuffer->Width, 1);
            glClear(GL_COLOR_BUFFER_BIT);
            // left
            glScissor(0, 0, 1, frameBuffer->Height);
            glClear(GL_COLOR_BUFFER_BIT);
            // right
            glScissor(frameBuffer->Width - 1, 0, 1, frameBuffer->Height);
            glClear(GL_COLOR_BUFFER_BIT);
        }
        ovrFramebuffer_Resolve(frameBuffer);
        frameParms.Layers[VRAPI_FRAME_LAYER_TYPE_WORLD].Textures[i].ColorTextureSwapChain = frameBuffer->ColorTextureSwapChain;
        frameParms.Layers[VRAPI_FRAME_LAYER_TYPE_WORLD].Textures[i].TextureSwapChainIndex = frameBuffer->TextureSwapChainIndex;
        frameParms.Layers[VRAPI_FRAME_LAYER_TYPE_WORLD].Textures[i].TexCoordsFromTanAngles = ovrMatrix4f_TanAngleMatrixFromProjection(&_projection);
        frameParms.Layers[VRAPI_FRAME_LAYER_TYPE_WORLD].Textures[i].HeadPose = _headTracker->getTracking().HeadPose;
        ovrFramebuffer_Advance(frameBuffer);
    }
    glDisable(GL_SCISSOR_TEST);
    
    glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
    ovrFramebuffer_SetNone();
    vrapi_SubmitFrame(_ovr, &frameParms);
}

NS_CC_END