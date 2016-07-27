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
#include "oculus/ovr/OVR_CAPI_GL.h"
#include "oculus/ovr/Extras/OVR_Math.h"

#define EYE_NUM 2

struct DepthBuffer;
struct TextureBuffer;

NS_CC_BEGIN

class Camera;
class Sprite;
class VROculusHeadTracker;

class VROculusRenderer : public VRIRenderer
{
public:
    VROculusRenderer();
    virtual ~VROculusRenderer();

    virtual void setup(GLView* glview);
    virtual void cleanup();
    virtual void render(Scene* scene, Renderer* renderer);
    virtual VRIHeadTracker* getHeadTracker();

protected:

    TextureBuffer   *_eyeRenderTexture[EYE_NUM];
    DepthBuffer     *_eyeDepthBuffer[EYE_NUM];
    ovrEyeRenderDesc _eyeRenderDesc[EYE_NUM];
    Mat4             _eyeProjections[EYE_NUM];
    GLuint           _mirrorFBO;
    ovrGLTexture    *_mirrorTexture;
    ovrHmd           _HMD;
    ovrLayerEyeFov   _ld;
    VROculusHeadTracker *_headTracker;
};

NS_CC_END
