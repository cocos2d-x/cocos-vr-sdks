
const char* ccCameraClearMalivrFrag = STRINGIFY(

\n#ifdef GL_ES\n
in mediump vec2 v_texCoord;
in mediump vec3 v_color;
\n#else\n
in vec2 v_texCoord;
in vec3 v_color;
\n#endif\n
layout(location=0) out vec4 fragColor;
void main()
{
    fragColor = vec4(v_color, 1.0);
}
);
