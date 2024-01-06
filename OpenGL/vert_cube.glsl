#version 120

attribute vec3 inVertex;
attribute vec3 inNormal;

uniform mat4 modelingMat2;
uniform mat4 modelingMatInvTr2;
uniform mat4 perspectiveMat2;

varying vec4 fragPos2;
varying vec3 N2;

void main(void)
{
    vec4 p = modelingMat2 * vec4(inVertex, 1); // translate to world coordinates
    vec3 Nw = vec3(modelingMatInvTr2 * vec4(inNormal, 0)); // provided by the programmer

    N2 = normalize(Nw);
    fragPos2 = p;

    // Calculate the model-view-projection matrix
    mat4 MVP = perspectiveMat2*modelingMat2;
    gl_Position = MVP * vec4(inVertex, 1);
}

