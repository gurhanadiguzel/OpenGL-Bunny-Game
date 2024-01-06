vec3 lightPos = vec3(0, 0, 20);

vec3 I = vec3(2, 2, 2);
vec3 Iamb = vec3(0.8, 0.8, 0.8);

vec3 ka = vec3(0.1, 0.1, 0.1);
vec3 ks = vec3(0.8, 0.8, 0.8);

uniform vec3 kd ;
uniform vec3 cameraPos2;

varying vec4 fragPos2;
varying vec3 N2;

void main(void)
{
    vec3 L = normalize(lightPos - vec3(fragPos2));
    vec3 V = normalize(cameraPos2 - vec3(fragPos2));
    vec3 H = normalize(L + V);
    float NdotL = max(dot(N2, L), 0.0);
    float NdotH = max(dot(N2, H), 0.0);

    // Lambertian (diffuse) reflection
    vec3 diffuseColor = I * kd * NdotL;

    // Blinn-Phong (specular) reflection
    float shininess = 20.0;  // Adjust the shininess value as needed
    vec3 specularColor = I * ks * pow(NdotH, shininess);

    // Ambient reflection
    vec3 ambientColor = Iamb * ka;

    // Final color calculation
    vec3 finalColor = ambientColor + diffuseColor + specularColor;

    // Set the output color to the sum of diffuse, ambient, and specular
    gl_FragColor = vec4(finalColor, 1.0);
    //gl_FragDepth = 0.5;
}
