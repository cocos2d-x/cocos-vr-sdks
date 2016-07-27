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
#include "deepoon/include/deepoon_sdk_native.h"
#include "deepoon/include/deepoon_sdk_utils.h"
#include "platform/android/jni/JniHelper.h"

#define EYE_NUM 2

typedef struct
{
    int						Width;
    int						Height;
    int						Multisamples;
    int						TextureSwapNum;
    int						TextureSwapIndex;
    GLuint                  *texIDs;
    GLuint				    *DepthBuffers;
    GLuint				    *FrameBuffers;
} dpnnFramebuffer;

NS_CC_BEGIN

class Camera;
class Sprite;
class VRDeepoonHeadTracker;

class CC_DLL VRDeepoonRenderer : public VRIRenderer
{
public:
    VRDeepoonRenderer();
    virtual ~VRDeepoonRenderer();

    virtual void setup(GLView* glview);
    virtual void cleanup();
    virtual void render(Scene* scene, Renderer* renderer);
    virtual VRIHeadTracker* getHeadTracker();
    
protected:
    
    dpnnFramebuffer _frameBuffer[EYE_NUM];
    Mat4            _eyeProjection;
    dpnnInstance    _instance;
    VRDeepoonHeadTracker *_headTracker;
};

NS_CC_END
