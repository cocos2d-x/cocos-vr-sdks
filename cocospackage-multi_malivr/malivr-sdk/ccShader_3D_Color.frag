
const char* cc3D_Color_malivr_frag = STRINGIFY(

\n#ifdef GL_ES\n
in lowp vec4 DestinationColor;
\n#else\n
in vec4 DestinationColor;
\n#endif\n
uniform vec4 u_color;
layout(location=0) out vec4 fragColor;
                                        
void main(void)
{
    fragColor = u_color;
}
);
