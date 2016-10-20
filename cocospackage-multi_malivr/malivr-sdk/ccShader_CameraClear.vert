
const char* ccCameraClearMalivrVert = STRINGIFY(
layout(num_views = 4) in;
uniform float depth;

layout (location = 0)in vec3 a_position;
layout (location = 1)in vec4 a_color;
layout (location = 2)in vec2 a_texCoord;
\n#ifdef GL_ES\n
out mediump vec2 v_texCoord;
out mediump vec3 v_color;
\n#else\n
out vec2 v_texCoord;
out vec3 v_color;
\n#endif\n
void main()
{
    gl_Position = vec4(a_position.xy, depth, 1.0);
    v_texCoord = a_texCoord;
    v_color = a_color.rgb;
}
);
