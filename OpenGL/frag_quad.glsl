#version 330

varying vec4 fragPos;
varying vec3 N;

uniform float offsetX;
uniform float offsetZ;

uniform float scaleX;
uniform float scaleZ;

void main(void)
{
    // Calculate the checkerboard pattern
    bool x = int((fragPos.x + offsetX) * scaleX) % 2  == 0;
    bool z = int((fragPos.z + offsetZ) * scaleZ) % 2  == 0;
    bool checkerPattern = x != z;

    vec3 color1 = vec3(0.467, 0.545, 0.647); 
    vec3 color2 = vec3(1.0, 1.0, 1.0);
    vec3 finalColor;
    if (checkerPattern) {
        finalColor = color1;
    } else {
        finalColor = color2;
    }

    gl_FragColor = vec4(finalColor, 1.0);

    //gl_FragDepth = 0.7;
}
