const char* ccPositionTexture_GrayScale_malivr_frag = STRINGIFY(

\n#ifdef GL_ES\n
precision mediump float;
\n#endif\n
in vec4 v_fragmentColor;
in vec2 v_texCoord;
layout(location=0) out vec4 fragColor;
void main(void)
{
	vec4 c = texture(CC_Texture0, v_texCoord);
     c = v_fragmentColor * c;
	fragColor.xyz = vec3(0.2126*c.r + 0.7152*c.g + 0.0722*c.b);
	fragColor.w = c.w;
}
);
