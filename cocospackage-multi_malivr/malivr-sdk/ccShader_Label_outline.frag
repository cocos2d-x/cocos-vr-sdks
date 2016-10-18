/*
 * LICENSE ???
 */
const char* ccLabelOutline_malivr_frag = STRINGIFY(
\n#ifdef GL_ES\n
precision lowp float; 
\n#endif\n
 
in vec4 v_fragmentColor;
in vec2 v_texCoord;

uniform vec4 u_effectColor;
uniform vec4 u_textColor;
layout(location=0) out vec4 fragColor;
                                            
void main()
{
    vec4 samples = texture(CC_Texture0, v_texCoord);
    float fontAlpha = samples.a;
    float outlineAlpha = samples.r; 
    if ((fontAlpha + outlineAlpha) > 0.0){
        vec4 color = u_textColor * fontAlpha + u_effectColor * (1.0 - fontAlpha);
        fragColor = v_fragmentColor * vec4( color.rgb,max(fontAlpha,outlineAlpha)*color.a);
    }
    else {
        discard;
    }
}
);
