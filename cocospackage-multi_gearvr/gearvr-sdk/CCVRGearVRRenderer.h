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

#include "vr/CCVRProtocol.h"
#include "renderer/CCCustomCommand.h"
#include "renderer/CCFrameBuffer.h"
#include "VrApi/Include/VrApi.h"
#include "VrApi/Include/VrApi_Helpers.h"

#define EYE_NUM VRAPI_FRAME_LAYER_EYE_MAX

typedef struct
{
    int						Width;
    int						Height;
    int						Multisamples;
    int						TextureSwapChainLength;
    int						TextureSwapChainIndex;
    ovrTextureSwapChain *	ColorTextureSwapChain;
    GLuint *				DepthBuffers;
    GLuint *				FrameBuffers;
} ovrFramebuffer;

NS_CC_BEGIN

class Camera;
class Sprite;
class VRGearVRHeadTracker;

class CC_DLL VRGearVRRenderer : public VRIRenderer
{
public:
    VRGearVRRenderer();
    virtual ~VRGearVRRenderer();

    virtual void setup(GLView* glview);
    virtual void cleanup();
    virtual void render(Scene* scene, Renderer* renderer);
    virtual VRIHeadTracker* getHeadTracker();
    
protected:
    
    ovrFramebuffer _frameBuffer[EYE_NUM];
    ovrMatrix4f    _projection;
    Mat4           _eyeProjection;
    ovrJava    _java;
    ovrMobile *_ovr;
    long long  _frameIndex;
    VRGearVRHeadTracker *_headTracker;
};

NS_CC_END
