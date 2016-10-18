
const char* cc3D_Terrain_malivr_vert = STRINGIFY(
layout(num_views = 4) in;                                          
in vec4 a_position;
in vec2 a_texCoord;
in vec3 a_normal;
\n#ifdef GL_ES\n
out mediump vec2 v_texCoord;
out mediump vec3 v_normal;
\n#else\n
out vec2 v_texCoord;
out vec3 v_normal;
\n#endif\n
void main()
{
    gl_Position = CC_MultiViewMVPMatrix[gl_ViewID_OVR] * a_position;
    v_texCoord = a_texCoord;
    v_normal = a_normal;
}
);
