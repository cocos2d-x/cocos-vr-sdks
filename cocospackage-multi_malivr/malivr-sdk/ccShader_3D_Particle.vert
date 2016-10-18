
const char* cc3D_Particle_malivr_vert = STRINGIFY(
layout(num_views = 4) in;                                              
in vec4 a_position;
in vec4 a_color;
in vec2 a_texCoord;

out vec2 TextureCoordOut;
out vec4 ColorOut;
void main()
{
    ColorOut = a_color;
    TextureCoordOut = a_texCoord;
    TextureCoordOut.y = 1.0 - TextureCoordOut.y;
    gl_Position = CC_MultiViewPMatrix[gl_ViewID_OVR] * a_position;
}

);