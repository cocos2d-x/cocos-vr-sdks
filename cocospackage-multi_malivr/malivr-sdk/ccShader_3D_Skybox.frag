const char* cc3D_Skybox_malivr_frag = STRINGIFY(
\n#ifdef GL_ES\n
in mediump vec3        v_reflect;
\n#else\n
in vec3        v_reflect;
\n#endif\n
uniform samplerCube u_Env;
uniform vec4 u_color;
layout(location=0) out vec4 fragColor;
void main(void)
{
    fragColor = texture(u_Env, v_reflect) * u_color;
}
);
