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

NS_CC_BEGIN

#define Meter(x) (x)
#define Centimeter(x) (Meter(x) / 100.0f)
#define Millimeter(x) (Meter(x) / 1000.0f)

// The following compile-time constants are used to calibrate the
// VR experience for each device and each user. For the optimal
// experience, you will need to perform measurements on your own
// device, head-mounted-display and eyes. The default values are
// calibrated for a Samsung Note 4 for a Gear VR SM-R320 HMD.

#define Num_Eyes 2
#define Num_Views Num_Eyes * 2

// The dimensions of the device screen in pixels and meters.
#define Screen_Resolution_X 2560
#define Screen_Resolution_Y 1440
#define Screen_Size_X       Meter(0.125f)
#define Screen_Size_Y       Meter(0.072f)

// The dimensions of the framebuffers used for both eyes when
// rendering the scene. The values for these will balance visual
// quality and performance. The framebuffers will be scaled down
// or up to fit inside the viewports.
#define Eye_Fb_Resolution_X (Screen_Resolution_X / 2)
#define Eye_Fb_Resolution_Y 1440

// If multisampling is available on the device the framebuffers
// will be rendered to using multisampling.
#define Multisample_Samples 4

// The interpupillary distance (IPD) is the distance between
// your pupils when looking straight ahead. The human average
// is about 64mm, which is the same as the distance between the
// lenses of the Gear VR headset. The user should set these to
// their own measured IPD for the most comfortable experience.
#define Eye_IPD Millimeter(61.0f)

// This should be set equal to the distance between the lens
// centres in the head-mounted display.
#define Lens_IPD Millimeter(64.0f)

// This should be set equal to the distance between the display
// and the center point of the viewer's eye.
#define Eye_Display_Distance Centimeter(3.0f)

// Defining border color enums in case the headers are not up to date for using the android extension pack.
#ifndef GL_TEXTURE_BORDER_COLOR_EXT
#define GL_TEXTURE_BORDER_COLOR_EXT 0x1004
#endif

#ifndef GL_CLAMP_TO_BORDER_EXT
#define GL_CLAMP_TO_BORDER_EXT 0x812D
#endif

struct Framebuffer
{
    int width;
    int height;
    GLuint framebuffer;
    GLuint depthbuffer;
    GLuint colorbuffer;
};

// These coefficients control the degree of distortion that
// is applied on the rendertargets, per channel. The notation
// is the same as in the original Brown-Conray distortion
// correction model.
struct DistortionCoefficients
{
    // Radial distortion coefficients
    float k1; // Central
    float k2; // Edge
    float k3; // Fine
    
    // Tangential distortion coefficients
    float p1; // Horizontal
    float p2; // Vertical
};

struct LensConfig
{
    // One set for each channel, to handle chromatic aberration.
    DistortionCoefficients coefficients_red;
    DistortionCoefficients coefficients_green;
    DistortionCoefficients coefficients_blue;
    
    // The viewer may not look through the lens centre. This means
    // that we need to perform an asymmetrical barrel distortion,
    // centered at an offset given by the difference between the
    // viewer's eye seperation and the HMD lens seperation.
    Vec2 distort_centre;
    
    // Each eye should be looking at the centre of each produced
    // framebuffer image. To do this we must shift the result of
    // each framebuffer by an appropriate amount, such that when
    // the viewer looks straight ahead, the left pupil is in the
    // centre of the left image and the right pupil vice versa.
    Vec2 image_centre;
    
    // The distorted image will appear smaller on the screen,
    // depending on the values of the distortion coefficients.
    // We apply a shape-preserving upscale to make the distorted
    // image fit into the edges of the screen. The value of this
    // scale is somewhat arbitrarily chosen.
    float fill_scale;
};

struct HMDConfig
{
    LensConfig left;
    LensConfig right;
};

struct WarpMesh
{
    GLuint vbo;
    GLuint ibo;
    int index_count;
};

struct vrApp
{
    
    // Distortion shader
    GLuint program_distort;
    GLuint a_distort_position;
    GLuint a_distort_uv_red_low_res;
    GLuint a_distort_uv_green_low_res;
    GLuint a_distort_uv_blue_low_res;
    GLuint a_distort_uv_red_high_res;
    GLuint a_distort_uv_green_high_res;
    GLuint a_distort_uv_blue_high_res;
    GLuint u_distort_layer_index;
    GLuint u_distort_framebuffer;
    
    // Geometry
    GLuint vao;
    GLuint vbo_cube;
    GLuint warp_mesh[Num_Eyes];
    
    HMDConfig hmd;
    Framebuffer fb;
};


class Camera;
class Sprite;
class VRMaliVRHeadTracker;

class CC_DLL VRMaliVRRenderer : public VRIRenderer
{
public:
    VRMaliVRRenderer();
    virtual ~VRMaliVRRenderer();

    virtual void setup(GLView* glview);
    virtual void cleanup();
    virtual void render(Scene* scene, Renderer* renderer);
    virtual VRIHeadTracker* getHeadTracker();
    
protected:
    
    VRMaliVRHeadTracker *_headTracker;
    vrApp                _vrApp;
};

NS_CC_END
