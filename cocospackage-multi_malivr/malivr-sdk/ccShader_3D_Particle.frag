
const char* cc3D_Particle_tex_malivr_frag = STRINGIFY(

\n#ifdef GL_ES\n
in mediump vec2 TextureCoordOut;
in mediump vec4 ColorOut;
\n#else\n
in vec4 ColorOut;
in vec2 TextureCoordOut;
\n#endif\n
uniform vec4 u_color;
layout(location=0) out vec4 fragColor;
                                               
void main(void)
{
    fragColor = texture(CC_Texture0, TextureCoordOut) * ColorOut * u_color;
}
);

const char* cc3D_Particle_color_malivr_frag = STRINGIFY(
                                               
\n#ifdef GL_ES\n
in mediump vec4 ColorOut;
\n#else\n
in vec4 ColorOut;
\n#endif\n
uniform vec4 u_color;
layout(location=0) out vec4 fragColor;
                                                 
void main(void)
{
    fragColor = ColorOut * u_color;
}
);
