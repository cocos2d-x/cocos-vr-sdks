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

#ifndef __CC_VR_MALIVR_SHADERS_H__
#define __CC_VR_MALIVR_SHADERS_H__

#include "platform/CCGL.h"
#include "platform/CCPlatformMacros.h"

NS_CC_BEGIN

extern CC_DLL const GLchar * ccPosition_uColor_malivr_frag;
extern CC_DLL const GLchar * ccPosition_uColor_malivr_vert;

extern CC_DLL const GLchar * ccPositionColor_malivr_frag;
extern CC_DLL const GLchar * ccPositionColor_malivr_vert;

extern CC_DLL const GLchar * ccPositionColorTextureAsPointsize_malivr_vert;

extern CC_DLL const GLchar * ccPositionTexture_malivr_frag;
extern CC_DLL const GLchar * ccPositionTexture_malivr_vert;

extern CC_DLL const GLchar * ccPositionTextureA8Color_malivr_frag;
extern CC_DLL const GLchar * ccPositionTextureA8Color_malivr_vert;

extern CC_DLL const GLchar * ccPositionTextureColor_malivr_frag;
extern CC_DLL const GLchar * ccPositionTextureColor_malivr_vert;

extern CC_DLL const GLchar * ccPositionTextureColor_noMVP_malivr_frag;
extern CC_DLL const GLchar * ccPositionTextureColor_noMVP_malivr_vert;

extern CC_DLL const GLchar * ccPositionTextureColorAlphaTest_malivr_frag;

extern CC_DLL const GLchar * ccPositionTexture_uColor_malivr_frag;
extern CC_DLL const GLchar * ccPositionTexture_uColor_malivr_vert;

extern CC_DLL const GLchar * ccPositionColorLengthTexture_malivr_frag;
extern CC_DLL const GLchar * ccPositionColorLengthTexture_malivr_vert;

extern CC_DLL const GLchar * ccPositionTexture_GrayScale_malivr_frag;

extern CC_DLL const GLchar * ccLabelDistanceFieldNormal_malivr_frag;
extern CC_DLL const GLchar * ccLabelDistanceFieldGlow_malivr_frag;
extern CC_DLL const GLchar * ccLabelNormal_malivr_frag;
extern CC_DLL const GLchar * ccLabelOutline_malivr_frag;

extern CC_DLL const GLchar * ccLabel_malivr_vert;

extern CC_DLL const GLchar * cc3D_PositionTex_malivr_vert;
extern CC_DLL const GLchar * cc3D_SkinPositionTex_malivr_vert;
extern CC_DLL const GLchar * cc3D_ColorTex_malivr_frag;
extern CC_DLL const GLchar * cc3D_Color_malivr_frag;
extern CC_DLL const GLchar * cc3D_PositionNormalTex_malivr_vert;
extern CC_DLL const GLchar * cc3D_SkinPositionNormalTex_malivr_vert;
extern CC_DLL const GLchar * cc3D_ColorNormalTex_malivr_frag;
extern CC_DLL const GLchar * cc3D_ColorNormal_malivr_frag;
extern CC_DLL const GLchar * cc3D_Particle_malivr_vert;
extern CC_DLL const GLchar * cc3D_Particle_tex_malivr_frag;
extern CC_DLL const GLchar * cc3D_Particle_color_malivr_frag;
extern CC_DLL const GLchar * cc3D_Skybox_malivr_vert;
extern CC_DLL const GLchar * cc3D_Skybox_malivr_frag;
extern CC_DLL const GLchar * cc3D_Terrain_malivr_vert;
extern CC_DLL const GLchar * cc3D_Terrain_malivr_frag;
extern CC_DLL const GLchar * ccCameraClearMalivrVert;
extern CC_DLL const GLchar * ccCameraClearMalivrFrag;
// ETC1 ALPHA supports.
extern CC_DLL const GLchar* ccETC1ASPositionTextureColor_malivr_frag;
extern CC_DLL const char* ccETC1ASPositionTextureGray_malivr_frag;
NS_CC_END
#endif
