
const char* cc3D_ColorTex_malivr_frag = STRINGIFY(

\n#ifdef GL_ES\n
in mediump vec2 TextureCoordOut;
\n#else\n
in vec2 TextureCoordOut;
\n#endif\n
uniform vec4 u_color;
layout(location=0) out vec4 fragColor;
                                           
void main(void)
{
    fragColor = texture(CC_Texture0, TextureCoordOut) * u_color;
}
);
