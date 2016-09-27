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
#include "CCVRMaliVRRenderer.h"
#include "CCVRMaliVRHeadTracker.h"
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

// These are the dimensions of the viewports (in pixels) used
// when rendering each eye's framebuffer.
#define View_Resolution_X (Screen_Resolution_X / 2)
#define View_Resolution_Y Screen_Resolution_Y

// These are used to ensure that the distortion appears
// circular, even when the quad is stretched across a
// non-square region of the device. Furthermore, we use
// them to fit the resulting distorted image such that
// the rendered scene fully fits into the viewport.
#define View_Aspect_Ratio ((float)View_Resolution_X / (float)View_Resolution_Y)
#define Eye_Fb_Aspect_Ratio ((float)Eye_Fb_Resolution_X / (float)Eye_Fb_Resolution_Y)

// The near-clipping plane does not need to be equal to the
// projection plane (the hmd). It can be set larger as long
// as you can ensure that no geometry gets clipped (since
// that is really jarring for users).
#define Z_Near (Eye_Display_Distance)
#define Z_Far  Meter(12.0f)

// Instead of recomputing the distortion per frame, we store
// the distorted texel lookup coordinates in the attributes of
// a tessellated quad. The coordinates are linearly interpolated
// between each vertex. This gives acceptable results given a
// high enough resolution of the mesh, even though the distortion
// equations are nonlinear.
#define Warp_Mesh_Resolution_X 64
#define Warp_Mesh_Resolution_Y 64

static const char* DISTORTION_V_SHADER = " \
#version 300 es      \n \
// This proprietary software may be used only as      \n \
// authorised by a licensing agreement from ARM Limited    \n \
// (C) COPYRIGHT 2015 ARM Limited      \n \
// ALL RIGHTS RESERVED      \n \
// The entire notice above must be reproduced on all authorised   \n \
// copies and copies may only be made to the extent permitted   \n \
// by a licensing agreement from ARM Limited.   \n \
\n \
in vec2 position;            \n \
in vec2 uv_red_low_res;      \n \
in vec2 uv_green_low_res;    \n \
in vec2 uv_blue_low_res;     \n \
in vec2 uv_red_high_res;     \n \
in vec2 uv_green_high_res;   \n \
in vec2 uv_blue_high_res;    \n \
out vec2 texel_r_low_res;    \n \
out vec2 texel_g_low_res;    \n \
out vec2 texel_b_low_res;    \n \
out vec2 texel_r_high_res;   \n \
out vec2 texel_g_high_res;   \n \
out vec2 texel_b_high_res;   \n \
 \n \
void main()                                   \n \
{                                             \n \
    gl_Position = vec4(position, 0.0, 1.0);   \n \
    texel_r_low_res = uv_red_low_res;         \n \
    texel_g_low_res = uv_green_low_res;       \n \
    texel_b_low_res = uv_blue_low_res;        \n \
    texel_r_high_res = uv_red_high_res;       \n \
    texel_g_high_res = uv_green_high_res;     \n \
    texel_b_high_res = uv_blue_high_res;      \n \
}";

static const char* DISTORTION_F_SHADER = " \
#version 300 es    \n \
// This proprietary software may be used only as   \n \
// authorised by a licensing agreement from ARM Limited   \n \
// (C) COPYRIGHT 2015 ARM Limited  \n \
// ALL RIGHTS RESERVED   \n \
// The entire notice above must be reproduced on all authorised  \n \
// copies and copies may only be made to the extent permitted  \n \
// by a licensing agreement from ARM Limited.  \n \
precision highp float;    \n \
precision mediump int;    \n \
precision mediump sampler2DArray;  \n \
\n \
in vec2 texel_r_low_res;            \n \
in vec2 texel_g_low_res;            \n \
in vec2 texel_b_low_res;            \n \
in vec2 texel_r_high_res;           \n \
in vec2 texel_g_high_res;           \n \
in vec2 texel_b_high_res;           \n \
uniform sampler2DArray framebuffer; \n \
uniform int layer_index;            \n \
out vec4 f_color;                   \n \
\n \
vec3 sample_per_channel(vec2 tex_coord_r, vec2 tex_coord_g, vec2 tex_coord_b, int layer) \n \
{  \n \
    vec3 sampled_color;   \n \
    sampled_color.r = texture(framebuffer, vec3(tex_coord_r, layer)).r;  \n \
    sampled_color.g = texture(framebuffer, vec3(tex_coord_g, layer)).g;  \n \
    sampled_color.b = texture(framebuffer, vec3(tex_coord_b, layer)).b;  \n \
    \n \
    return sampled_color; \n \
} \n \
\n \
float interpolate_color(vec2 tex_coord, float low_res_color_val, float high_res_color_val) \n \
{\n \
    // Using squared distance to middle of screen for interpolating. \n \
    vec2 dist_vec = vec2(0.5) - tex_coord; \n \
    float squared_dist = dot(dist_vec, dist_vec); \n \
    // Using the high res texture when distance from center is less than 0.5 in texture coordinates (0.25 is 0.5 squared). \n \
    // When the distance is less than 0.2 (0.04 is 0.2 squared), only the high res texture will be used. \n \
    float lerp_val = smoothstep(-0.25, -0.4, -squared_dist); \n \
    return mix(low_res_color_val, high_res_color_val, lerp_val); \n \
}\n \
\n \
void main()\n \
{\n \
    vec3 low_res_color = sample_per_channel(texel_r_low_res, texel_g_low_res, texel_b_low_res, layer_index);   \n \
    vec3 high_res_color = sample_per_channel(texel_r_high_res, texel_g_high_res, texel_b_high_res, layer_index + 2);   \n \
    \n \
    f_color.r = interpolate_color(texel_r_high_res, low_res_color.r, high_res_color.r);  \n \
    f_color.g = interpolate_color(texel_g_high_res, low_res_color.g, high_res_color.g);  \n \
    f_color.b = interpolate_color(texel_b_high_res, low_res_color.b, high_res_color.b);  \n \
    f_color.a = 1.0;  \n \
}";


NS_CC_BEGIN

GLuint compile_shader(const char *source, GLenum type)
{
    GLuint result = glCreateShader(type);
    glShaderSource(result, 1, (const GLchar**)&source, NULL);
    glCompileShader(result);
    GLint status;
    glGetShaderiv(result, GL_COMPILE_STATUS, &status);
    if (status == GL_FALSE) {
        GLint length;
        glGetShaderiv(result, GL_INFO_LOG_LENGTH, &length);
        GLchar *info = new GLchar[length];
        glGetShaderInfoLog(result, length, NULL, info);
        CCLOG("[COMPILE] %s\n", info);
        delete[] info;
        exit(1);
    }
    return result;
}

GLuint link_program(GLuint *shaders, int count)
{
    GLuint program = glCreateProgram();
    for (int i = 0; i < count; ++i)
        glAttachShader(program, shaders[i]);
    
    glLinkProgram(program);
    
    for (int i = 0; i < count; ++i)
        glDetachShader(program, shaders[i]);
    
    GLint status;
    glGetProgramiv(program, GL_LINK_STATUS, &status);
    if (status == GL_FALSE) {
        GLint length;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &length);
        GLchar *info = new GLchar[length];
        glGetProgramInfoLog(program, length, NULL, info);
        CCLOG("[LINK] %s\n", info);
        delete[] info;
        exit(1);
    }
    return program;
}

Framebuffer make_eye_framebuffer(int width, int height, int num_views)
{
    Framebuffer result = {};
    //    result.width = width;
    //    result.height = height;
    //
    //    PFNGLFRAMEBUFFERTEXTUREMULTIVIEWOVR glFramebufferTextureMultiviewOVR =
    //    (PFNGLFRAMEBUFFERTEXTUREMULTIVIEWOVR)eglGetProcAddress ("glFramebufferTextureMultiviewOVR");
    //    PFNGLFRAMEBUFFERTEXTUREMULTISAMPLEMULTIVIEWOVR glFramebufferTextureMultisampleMultiviewOVR =
    //    (PFNGLFRAMEBUFFERTEXTUREMULTISAMPLEMULTIVIEWOVR)eglGetProcAddress ("glFramebufferTextureMultisampleMultiviewOVR");
    //
    //    if (!glFramebufferTextureMultiviewOVR)
    //    {
    //        CCLOG("Did not have glFramebufferTextureMultiviewOVR\n");
    //        return result;
    //    }
    //    if (!glFramebufferTextureMultisampleMultiviewOVR)
    //    {
    //        CCLOG("Did not have glFramebufferTextureMultisampleMultiviewOVR\n");
    //    }
    //
    //    bool have_multisampled_ext = glFramebufferTextureMultisampleMultiviewOVR != 0;
    //
    //    glGenFramebuffers(1, &result.framebuffer);
    //    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, result.framebuffer);
    //
    //    glGenTextures(1, &result.depthbuffer);
    //    glBindTexture(GL_TEXTURE_2D_ARRAY, result.depthbuffer);
    //    glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, GL_DEPTH_COMPONENT16, width, height, num_views);
    //
    //    if (have_multisampled_ext)
    //    {
    //        glFramebufferTextureMultisampleMultiviewOVR(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, result.depthbuffer, 0, Multisample_Samples, 0, num_views);
    //    }
    //    else
    //    {
    //        glFramebufferTextureMultiviewOVR(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, result.depthbuffer, 0, 0, num_views);
    //    }
    //
    //    glGenTextures(1, &result.colorbuffer);
    //    glBindTexture(GL_TEXTURE_2D_ARRAY, result.colorbuffer);
    //    glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, GL_RGBA8, width, height, num_views);
    //    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    //    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    //    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER_EXT);
    //    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER_EXT);
    //    GLint border_color[4] = {0, 0, 0, 0};
    //    glTexParameteriv(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_BORDER_COLOR_EXT, border_color);
    //    glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
    //
    //    if (have_multisampled_ext)
    //    {
    //        glFramebufferTextureMultisampleMultiviewOVR(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, result.colorbuffer, 0, Multisample_Samples, 0, num_views);
    //    }
    //    else
    //    {
    //        glFramebufferTextureMultiviewOVR(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, result.colorbuffer, 0, 0, num_views);
    //    }
    //
    //    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    //    if (status != GL_FRAMEBUFFER_COMPLETE)
    //        CCLOG("Framebuffer not complete\n");
    //    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    
    return result;
}

// Computes the distorted texel coordinate given the
// position on the image plane.
Vec2 compute_distortion(float x, float y,
                        Vec2 distort_centre,
                        DistortionCoefficients coefficients,
                        float tex_coord_factor)
{
    float k1 = coefficients.k1;
    float k2 = coefficients.k2;
    float k3 = coefficients.k3;
    float p1 = coefficients.p1;
    float p2 = coefficients.p2;
    
    // We need to correct for aspect ratio to ensure that
    // the distortion appears circular on the device.
    y /= View_Aspect_Ratio;
    
    float dx = x - distort_centre.x;
    float dy = y - distort_centre.y;
    float r2 = dx * dx + dy * dy;
    float r4 = r2 * r2;
    float r6 = r4 * r2;
    
    float radial_x = x * (k1 * r2 + k2 * r4 + k3 * r6);
    float radial_y = y * (k1 * r2 + k2 * r4 + k3 * r6);
    
    float tangential_x = p1 * (r2 + 2.0f*x*x) + 2.0f*p2*x*y;
    float tangential_y = p2 * (r2 + 2.0f*y*y) + 2.0f*p1*x*y;
    
    float distorted_x = x + radial_x + tangential_x;
    float distorted_y = y + radial_y + tangential_y;
    
    float result_x = 0.5f + tex_coord_factor * distorted_x;
    float result_y = 0.5f + tex_coord_factor * distorted_y * View_Aspect_Ratio;
    
    return Vec2(result_x, result_y);
}

GLuint make_warp_mesh(LensConfig config)
{
    struct Vertex
    {
        Vec2 position;
        Vec2 uv_red_low_res;
        Vec2 uv_green_low_res;
        Vec2 uv_blue_low_res;
        Vec2 uv_red_high_res;
        Vec2 uv_green_high_res;
        Vec2 uv_blue_high_res;
    };
    static Vertex v[(Warp_Mesh_Resolution_X + 1) * (Warp_Mesh_Resolution_Y + 1)];
    
    // Compute vertices
    int vi = 0;
    for (int yi = 0; yi <= Warp_Mesh_Resolution_Y; yi++)
        for (int xi = 0; xi <= Warp_Mesh_Resolution_X; xi++)
        {
            float x = -1.0f + 2.0f * xi / Warp_Mesh_Resolution_X;
            float y = -1.0f + 2.0f * yi / Warp_Mesh_Resolution_Y;
            v[vi].position = Vec2(x, y) * config.fill_scale + config.image_centre;
            v[vi].uv_red_low_res   = compute_distortion(x, y, config.distort_centre, config.coefficients_red, 0.5f);
            v[vi].uv_green_low_res = compute_distortion(x, y, config.distort_centre, config.coefficients_green, 0.5f);
            v[vi].uv_blue_low_res  = compute_distortion(x, y, config.distort_centre, config.coefficients_blue, 0.5f);
            // The texture coordinates for the higher resolution texture go from -0.5 to 1.5 so
            // that only the center of the screen samples the high resolution texture.
            v[vi].uv_red_high_res   = compute_distortion(x, y, config.distort_centre, config.coefficients_red, 1.0f);
            v[vi].uv_green_high_res = compute_distortion(x, y, config.distort_centre, config.coefficients_green, 1.0f);
            v[vi].uv_blue_high_res  = compute_distortion(x, y, config.distort_centre, config.coefficients_blue, 1.0f);
            vi++;
        }
    
    // Generate faces from vertices
    static Vertex f[Warp_Mesh_Resolution_X * Warp_Mesh_Resolution_Y * 6];
    int fi = 0;
    for (int yi = 0; yi < Warp_Mesh_Resolution_Y; yi++)
        for (int xi = 0; xi < Warp_Mesh_Resolution_X; xi++)
        {
            Vertex v0 = v[(yi    ) * (Warp_Mesh_Resolution_X + 1) + xi    ];
            Vertex v1 = v[(yi    ) * (Warp_Mesh_Resolution_X + 1) + xi + 1];
            Vertex v2 = v[(yi + 1) * (Warp_Mesh_Resolution_X + 1) + xi + 1];
            Vertex v3 = v[(yi + 1) * (Warp_Mesh_Resolution_X + 1) + xi    ];
            f[fi++] = v0;
            f[fi++] = v1;
            f[fi++] = v2;
            f[fi++] = v2;
            f[fi++] = v3;
            f[fi++] = v0;
        }
    
    GLuint result = 0;
    glGenBuffers(1, &result);
    glBindBuffer(GL_ARRAY_BUFFER, result);
    glBufferData(GL_ARRAY_BUFFER, sizeof(f), f, GL_STATIC_DRAW);
    return result;
}

VRMaliVRRenderer::VRMaliVRRenderer()
{
    _headTracker = new VRMaliVRHeadTracker;
}

VRMaliVRRenderer::~VRMaliVRRenderer()
{
    CC_SAFE_DELETE(_headTracker);
}

void VRMaliVRRenderer::setup(GLView* glview)
{
    // Make sure the required extensions are present.
    const GLubyte* extensions = glGetString(GL_EXTENSIONS);
    char * found_multiview2_extension = strstr ((const char*)extensions, "GL_OVR_multiview2");
    char * found_multisample_multiview_extension = strstr ((const char*)extensions, "GL_OVR_multiview_multisampled_render_to_texture");
    char * found_border_clamp_extension = strstr ((const char*)extensions, "GL_EXT_texture_border_clamp");
    
    if (found_multiview2_extension == NULL)
    {
        CCLOG("OpenGL ES 3.0 implementation does not support GL_OVR_multiview2 extension.\n");
    }
    
    if (found_multisample_multiview_extension == NULL)
    {
        // If multisampled multiview is not supported, multisampling will not be used, so no need to exit here.
        CCLOG("OpenGL ES 3.0 implementation does not support GL_OVR_multiview_multisampled_render_to_texture extension.\n");
    }
    
    if (found_border_clamp_extension == NULL)
    {
        CCLOG("OpenGL ES 3.0 implementation does not support GL_EXT_texture_border_clamp extension.\n");
    }
    
    glGenVertexArrays(1, &_vrApp.vao);
    glBindVertexArray(_vrApp.vao);
    
    _vrApp.fb = make_eye_framebuffer(Eye_Fb_Resolution_X, Eye_Fb_Resolution_Y, Num_Views);
    
    // The coefficients below may be calibrated by photographing an
    // image containing straight lines, both vertical and horizontal,
    // through the lenses of the HMD, at the position where the viewer
    // would be looking through them.
    
    // Ideally, the user would be allowed to calibrate them for their
    // own eyes, through some calibration utility. The application should
    // then load a stored user-profile on runtime. For now, we hardcode
    // some values based on our calibration of the SM-R320 Gear VR
    // lenses.
    
    // Left lens
    _vrApp.hmd.left.coefficients_red.k1    = 0.19f;
    _vrApp.hmd.left.coefficients_red.k2    = 0.21f;
    _vrApp.hmd.left.coefficients_red.k3    = 0.0f;
    _vrApp.hmd.left.coefficients_red.p1    = 0.0f;
    _vrApp.hmd.left.coefficients_red.p2    = 0.0f;
    _vrApp.hmd.left.coefficients_green.k1  = 0.22f;
    _vrApp.hmd.left.coefficients_green.k2  = 0.24f;
    _vrApp.hmd.left.coefficients_green.k3  = 0.0f;
    _vrApp.hmd.left.coefficients_green.p1  = 0.0f;
    _vrApp.hmd.left.coefficients_green.p2  = 0.0f;
    _vrApp.hmd.left.coefficients_blue.k1   = 0.24f;
    _vrApp.hmd.left.coefficients_blue.k2   = 0.26f;
    _vrApp.hmd.left.coefficients_blue.k3   = 0.0f;
    _vrApp.hmd.left.coefficients_blue.p1   = 0.0f;
    _vrApp.hmd.left.coefficients_blue.p2   = 0.0f;
    
    // Right lens
    _vrApp.hmd.right.coefficients_red.k1   = 0.19f;
    _vrApp.hmd.right.coefficients_red.k2   = 0.21f;
    _vrApp.hmd.right.coefficients_red.k3   = 0.0f;
    _vrApp.hmd.right.coefficients_red.p1   = 0.0f;
    _vrApp.hmd.right.coefficients_red.p2   = 0.0f;
    _vrApp.hmd.right.coefficients_green.k1 = 0.22f;
    _vrApp.hmd.right.coefficients_green.k2 = 0.24f;
    _vrApp.hmd.right.coefficients_green.k3 = 0.0f;
    _vrApp.hmd.right.coefficients_green.p1 = 0.0f;
    _vrApp.hmd.right.coefficients_green.p2 = 0.0f;
    _vrApp.hmd.right.coefficients_blue.k1  = 0.24f;
    _vrApp.hmd.right.coefficients_blue.k2  = 0.26f;
    _vrApp.hmd.right.coefficients_blue.k3  = 0.0f;
    _vrApp.hmd.right.coefficients_blue.p1  = 0.0f;
    _vrApp.hmd.right.coefficients_blue.p2  = 0.0f;
    
    // These may be computed by measuring the distance between the top
    // of the unscaled distorted image and the top of the screen. Denote
    // this distance by Delta. The normalized view coordinate of the
    // distorted image top is
    //     Y = 1 - 2 Delta / Screen_Size_Y
    // We want to scale this coordinate such that it maps to the top of
    // the view. That is,
    //     Y * fill_scale = 1
    // Solving for fill_scale gives the equations below.
    float delta = Centimeter(0.7f);
    _vrApp.hmd.left.fill_scale  = 1.0f / (1.0f - 2.0f * delta / Screen_Size_Y);
    _vrApp.hmd.right.fill_scale = 1.0f / (1.0f - 2.0f * delta / Screen_Size_Y);
    
    // These are computed such that the centers of the displayed framebuffers
    // on the device are seperated by the viewer's IPD.
    _vrApp.hmd.left.image_centre    = Vec2(+1.0f - Eye_IPD / (Screen_Size_X / 2.0f), 0.0f);
    _vrApp.hmd.right.image_centre   = Vec2(-1.0f + Eye_IPD / (Screen_Size_X / 2.0f), 0.0f);
    
    // These are computed such that the distortion takes place around
    // an offset determined by the difference between lens seperation
    // and the viewer's eye IPD. If the difference is zero, the distortion
    // takes place around the image centre.
    _vrApp.hmd.left.distort_centre  = Vec2((Lens_IPD - Eye_IPD) / (Screen_Size_X / 2.0f), 0.0f);
    _vrApp.hmd.right.distort_centre = Vec2((Eye_IPD - Lens_IPD) / (Screen_Size_X / 2.0f), 0.0f);
    
    _vrApp.warp_mesh[0] = make_warp_mesh(_vrApp.hmd.left);
    _vrApp.warp_mesh[1] = make_warp_mesh(_vrApp.hmd.right);
    
    GLuint shaders[2];
    shaders[0] = compile_shader(DISTORTION_V_SHADER, GL_VERTEX_SHADER);
    shaders[1] = compile_shader(DISTORTION_F_SHADER, GL_FRAGMENT_SHADER);
    _vrApp.program_distort = link_program(shaders, 2);
    
    _vrApp.a_distort_position          = glGetAttribLocation(_vrApp.program_distort, "position");
    _vrApp.a_distort_uv_red_low_res    = glGetAttribLocation(_vrApp.program_distort, "uv_red_low_res");
    _vrApp.a_distort_uv_green_low_res  = glGetAttribLocation(_vrApp.program_distort, "uv_green_low_res");
    _vrApp.a_distort_uv_blue_low_res   = glGetAttribLocation(_vrApp.program_distort, "uv_blue_low_res");
    _vrApp.a_distort_uv_red_high_res   = glGetAttribLocation(_vrApp.program_distort, "uv_red_high_res");
    _vrApp.a_distort_uv_green_high_res = glGetAttribLocation(_vrApp.program_distort, "uv_green_high_res");
    _vrApp.a_distort_uv_blue_high_res  = glGetAttribLocation(_vrApp.program_distort, "uv_blue_high_res");
    
    _vrApp.u_distort_layer_index = glGetUniformLocation(_vrApp.program_distort, "layer_index");
    _vrApp.u_distort_framebuffer = glGetUniformLocation(_vrApp.program_distort, "framebuffer");
    
//    get_attrib_location (cube, position);
//    get_attrib_location (cube, normal);
//    get_uniform_location(cube, projection);
//    get_uniform_location(cube, view);
//    get_uniform_location(cube, model);
}

void VRMaliVRRenderer::cleanup()
{
}

VRIHeadTracker* cocos2d::VRMaliVRRenderer::getHeadTracker()
{
    return _headTracker;
}

void VRMaliVRRenderer::render(Scene* scene, Renderer* renderer)
{
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    
    glBindFramebuffer(GL_FRAMEBUFFER, _vrApp.fb.framebuffer);
    Camera::setDefaultViewport(experimental::Viewport(0, 0, _vrApp.fb.width, _vrApp.fb.height));
    glClearColor(0.125f, 0.125f, 0.125f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    //scene->render(renderer, transform.getInversed(), &_eyeProjection);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    
    ////////////////////////////
    // Distortion shader
    
    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    glUseProgram(_vrApp.program_distort);
    glActiveTexture(GL_TEXTURE0);
    glUniform1f(_vrApp.u_distort_framebuffer, 0);
    
#define attribfv(prog, name, n, stride, offset) \
glEnableVertexAttribArray(_vrApp.a_##prog##_##name); \
glVertexAttribPointer(_vrApp.a_##prog##_##name, n, GL_FLOAT, GL_FALSE, \
stride * sizeof(GLfloat), (void*)(offset * sizeof(GLfloat)));
    
    // Left eye
    glViewport(0, 0, View_Resolution_X, View_Resolution_Y);
    //glBindTexture(GL_TEXTURE_2D_ARRAY, _vrApp.fb.colorbuffer);
    glUniform1f(_vrApp.u_distort_layer_index, 0);
    
    glBindBuffer(GL_ARRAY_BUFFER, _vrApp.warp_mesh[0]);
    attribfv(distort, position,          2, 14, 0);
    attribfv(distort, uv_red_low_res,    2, 14, 2);
    attribfv(distort, uv_green_low_res,  2, 14, 4);
    attribfv(distort, uv_blue_low_res,   2, 14, 6);
    attribfv(distort, uv_red_high_res,   2, 14, 8);
    attribfv(distort, uv_green_high_res, 2, 14, 10);
    attribfv(distort, uv_blue_high_res,  2, 14, 12);
    glDrawArrays(GL_TRIANGLES, 0, Warp_Mesh_Resolution_X * Warp_Mesh_Resolution_Y * 6);
    
    // Right eye
    glViewport(View_Resolution_X, 0, View_Resolution_X, View_Resolution_Y);
    glUniform1f(_vrApp.u_distort_layer_index, 1);
    glBindBuffer(GL_ARRAY_BUFFER, _vrApp.warp_mesh[1]);
    attribfv(distort, position,          2, 14, 0);
    attribfv(distort, uv_red_low_res,    2, 14, 2);
    attribfv(distort, uv_green_low_res,  2, 14, 4);
    attribfv(distort, uv_blue_low_res,   2, 14, 6);
    attribfv(distort, uv_red_high_res,   2, 14, 8);
    attribfv(distort, uv_green_high_res, 2, 14, 10);
    attribfv(distort, uv_blue_high_res,  2, 14, 12);
    glDrawArrays(GL_TRIANGLES, 0, Warp_Mesh_Resolution_X * Warp_Mesh_Resolution_Y * 6);
}

NS_CC_END