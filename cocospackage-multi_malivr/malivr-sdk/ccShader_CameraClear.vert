
const char* ccCameraClearMalivrVert = STRINGIFY(
layout(num_views = 4) in;
uniform float depth;

in vec4 a_position;
in vec2 a_texCoord;
in vec3 a_color;
\n#ifdef GL_ES\n
out mediump vec2 v_texCoord;
out mediump vec3 v_color;
\n#else\n
out vec2 v_texCoord;
out vec3 v_color;
\n#endif\n
void main()
{
    gl_Position = a_position;
    gl_Position.z = depth;
    gl_Position.w = 1.0;
    v_texCoord = a_texCoord;
    v_color = a_color;
}
);
