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
#include "CCVRDeepoonRenderer.h"
#include "CCVRDeepoonHeadTracker.h"
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

#define GL( func )		func;
#define NUM_MULTI_SAMPLES	4
const int EYE_BUFFER_SIZE = 1024;

static void dpnnFramebuffer_Clear(dpnnFramebuffer * frameBuffer)
{
    frameBuffer->Width = 0;
    frameBuffer->Height = 0;
    frameBuffer->Multisamples = 0;
    frameBuffer->TextureSwapNum = 0;
    frameBuffer->TextureSwapIndex = 0;
    frameBuffer->texIDs = nullptr;
    frameBuffer->DepthBuffers = nullptr;
    frameBuffer->FrameBuffers = nullptr;
}

static bool dpnnFramebuffer_Create(dpnnFramebuffer * frameBuffer, const int width, const int height, const int multisamples)
{
    frameBuffer->Width = width;
    frameBuffer->Height = height;
    frameBuffer->Multisamples = multisamples;
    
    frameBuffer->TextureSwapNum = 3;
    frameBuffer->texIDs = (GLuint *)malloc(frameBuffer->TextureSwapNum * sizeof(GLuint));
    frameBuffer->DepthBuffers = (GLuint *)malloc(frameBuffer->TextureSwapNum * sizeof(GLuint));
    frameBuffer->FrameBuffers = (GLuint *)malloc(frameBuffer->TextureSwapNum * sizeof(GLuint));
    
    //PFNGLRENDERBUFFERSTORAGEMULTISAMPLEEXTPROC glRenderbufferStorageMultisampleEXT =
    //	(PFNGLRENDERBUFFERSTORAGEMULTISAMPLEEXTPROC)eglGetProcAddress("glRenderbufferStorageMultisampleEXT");
    //PFNGLFRAMEBUFFERTEXTURE2DMULTISAMPLEEXTPROC glFramebufferTexture2DMultisampleEXT =
    //	(PFNGLFRAMEBUFFERTEXTURE2DMULTISAMPLEEXTPROC)eglGetProcAddress("glFramebufferTexture2DMultisampleEXT");
    
    for (int i = 0; i < frameBuffer->TextureSwapNum; i++) {
        // Create the color buffer texture.
        GL(glGenTextures(1, &frameBuffer->texIDs[i]));
        GL(glBindTexture(GL_TEXTURE_2D, frameBuffer->texIDs[i]));
        GL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0,
                        GL_RGBA, GL_UNSIGNED_BYTE, NULL));
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
        //	GL(glFramebufferTexture2DMultisampleEXT(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, frameBuffer->texIDs[i], 0, multisamples));
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
            // Create depth buffer.
            GL(glGenRenderbuffers(1, &frameBuffer->DepthBuffers[i]));
            GL(glBindRenderbuffer(GL_RENDERBUFFER, frameBuffer->DepthBuffers[i]));
            GL(glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24_OES, width, height));
            GL(glBindRenderbuffer(GL_RENDERBUFFER, 0));
            
            // Create the frame buffer.
            GL(glGenFramebuffers(1, &frameBuffer->FrameBuffers[i]));
            GL(glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer->FrameBuffers[i]));
            GL(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, frameBuffer->DepthBuffers[i]));
            GL(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, frameBuffer->texIDs[i], 0));
            GL(GLenum renderFramebufferStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER));
            GL(glBindFramebuffer(GL_FRAMEBUFFER, 0));
            if (renderFramebufferStatus != GL_FRAMEBUFFER_COMPLETE)
            {
                return false;
            }
        }
    }
    
    return true;
}

static void dpnnFramebuffer_Destroy(dpnnFramebuffer * frameBuffer)
{
    GL(glDeleteTextures(frameBuffer->TextureSwapNum, frameBuffer->texIDs));
    GL(glDeleteFramebuffers(frameBuffer->TextureSwapNum, frameBuffer->FrameBuffers));
    GL(glDeleteRenderbuffers(frameBuffer->TextureSwapNum, frameBuffer->DepthBuffers));
    
    free(frameBuffer->texIDs);
    free(frameBuffer->DepthBuffers);
    free(frameBuffer->FrameBuffers);
    
    dpnnFramebuffer_Clear(frameBuffer);
}

static void dpnnFramebuffer_SetCurrent(dpnnFramebuffer * frameBuffer)
{
    GL(glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer->FrameBuffers[frameBuffer->TextureSwapIndex]));
}

static void dpnnFramebuffer_SetNone()
{
    GL(glBindFramebuffer(GL_FRAMEBUFFER, 0));
}

static void dpnnFramebuffer_Resolve(dpnnFramebuffer * frameBuffer)
{
    // Discard the depth buffer, so the tiler won't need to write it back out to memory.
    const GLenum depthAttachment[1] = { GL_DEPTH_ATTACHMENT };
    //glInvalidateFramebuffer(GL_FRAMEBUFFER, 1, depthAttachment);
    
    // Flush this frame worth of commands.
    glFlush();
}

static void dpnnFramebuffer_Advance(dpnnFramebuffer * frameBuffer)
{
    // Advance to the next texture from the set.
    frameBuffer->TextureSwapIndex = (frameBuffer->TextureSwapIndex + 1) % frameBuffer->TextureSwapNum;
}

NS_CC_BEGIN

VRDeepoonRenderer::VRDeepoonRenderer()
    : _instance(nullptr)
{
    JniHelper::callStaticVoidMethod("java/lang/System", "loadLibrary", std::string("deepoon_sdk"));
    _headTracker = new VRDeepoonHeadTracker;
}

VRDeepoonRenderer::~VRDeepoonRenderer()
{
    CC_SAFE_DELETE(_headTracker);
}

void VRDeepoonRenderer::setup(GLView* glview)
{
    //JniHelper::callStaticVoidMethod("java/lang/System", "loadLibrary", std::string("deepoon_sdk"));
    //_instance = dpnnInit(1, DPNN_UM_DEFAULT, NULL, DPNN_DEVICE_GLES2, JniHelper::getActivity());
    //_headTracker->setdpnnInstance(&_instance);
    
    for (int eye = 0; eye < EYE_NUM; eye++)
    {
        dpnnFramebuffer_Clear(&_frameBuffer[eye]);
        dpnnFramebuffer_Create(&_frameBuffer[eye],
                               EYE_BUFFER_SIZE,
                               EYE_BUFFER_SIZE,
                               NUM_MULTI_SAMPLES);
    }

    auto backToForegroundListener = EventListenerCustom::create(EVENT_COME_TO_FOREGROUND,
                                                                [=](EventCustom*)
                                                                {
                                                                    if (!_instance){
                                                                        _instance = dpnnInit(1, DPNN_UM_DEFAULT, NULL, DPNN_DEVICE_GLES2, (void*)JniHelper::getActivity());
                                                                        _headTracker->setdpnnInstance(_instance);
                                                                        dpnnDeviceInfo dInfo;
                                                                        dpnnGetDeviceInfo(_instance, &dInfo);
                                                                        auto projection = dpnutilMatrix4_CreateProjectionFov(CC_DEGREES_TO_RADIANS(dInfo.fov_x), CC_DEGREES_TO_RADIANS(dInfo.fov_y), 0.0f, 0.0f, 0.1f, 5000.0f);
                                                                        _eyeProjection.set((const GLfloat *)(dpnutilMatrix4_Transpose(&projection).M[0]));
                                                                    }
                                                                }
                                                                );
    Director::getInstance()->getEventDispatcher()->addEventListenerWithFixedPriority(backToForegroundListener, -1);
    
    
    auto foregroundToBackListener = EventListenerCustom::create(EVENT_COME_TO_BACKGROUND,
                                                                [=](EventCustom*)
                                                                {
                                                                    if (_instance){
                                                                        _headTracker->setdpnnInstance(nullptr);
                                                                        dpnnDeinit(_instance);
                                                                        _instance = nullptr;
                                                                    }
                                                                }
                                                                );
    Director::getInstance()->getEventDispatcher()->addEventListenerWithFixedPriority(foregroundToBackListener, -1);
}

void VRDeepoonRenderer::cleanup()
{
    for (int eye = 0; eye < EYE_NUM; eye++)
    {
        dpnnFramebuffer_Destroy(&_frameBuffer[eye]);
    }
}

VRIHeadTracker* VRDeepoonRenderer::getHeadTracker()
{
    return _headTracker;
}

void VRDeepoonRenderer::render(Scene* scene, Renderer* renderer)
{
    if (!_instance) return;
    
    Mat4 headView = _headTracker->getLocalRotation();
    Mat4 transform;
    const dpnHmdParms headModelParms = dpnutilDefaultHmdParms();
    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);
    
    glEnable(GL_SCISSOR_TEST);
    for (unsigned short i = 0; i < EYE_NUM; ++i){
        dpnnFramebuffer * frameBuffer = &_frameBuffer[i];
        dpnnFramebuffer_SetCurrent(frameBuffer);
        int width = frameBuffer->Width;
        int height = frameBuffer->Height;
        
        Camera::setDefaultViewport(experimental::Viewport(0, 0, width, height));
        glScissor(0, 0, width, height);
        glClearColor(0.125f, 0.125f, 0.125f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        const float eyeOffset = ( i ? -0.5f : 0.5f ) * headModelParms.ipd;
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
        dpnnFramebuffer_Resolve(frameBuffer);
        dpnnSetTexture(_instance, (void*)(frameBuffer->texIDs[frameBuffer->TextureSwapIndex]), i == 0 ? DPNN_EYE_LEFT : DPNN_EYE_RIGHT, DPNN_TW_NONE);
        dpnnFramebuffer_Advance(frameBuffer);
    }
    glDisable(GL_SCISSOR_TEST);
    
    dpnnRecordPose(_instance, DPNN_EYE_COUNT);
    dpnnCompose(_instance);
    dpnnFramebuffer_SetNone();
    glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
}

NS_CC_END