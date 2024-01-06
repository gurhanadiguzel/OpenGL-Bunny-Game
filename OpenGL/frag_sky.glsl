#version 120

uniform sampler2D backgroundTexture;

void main()
{
    gl_FragColor = texture2D(backgroundTexture, gl_TexCoord[0].st);
    gl_FragDepth = 0.9999;
}
