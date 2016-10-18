const char* cc3D_Skybox_malivr_vert = STRINGIFY(
layout(num_views = 4) in;
uniform mat4  u_cameraRot;
in vec3 a_position;
out vec3 v_reflect;

void main(void)
{
    vec4 reflect =  u_cameraRot * vec4(a_position, 1.0);
    v_reflect = reflect.xyz;
    gl_Position = vec4(a_position.xy, 1.0 , 1.0);
}
);