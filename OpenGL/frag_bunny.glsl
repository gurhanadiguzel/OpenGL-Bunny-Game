vec3 lightPos = vec3(-5, 100, 100);
vec3 I = vec3(0.9, 0.9, 0.9);
vec3 Iamb = vec3(0.2, 0.2, 0.2);

vec3 kd = vec3(0.984, 0.549, 0.000); // Bunny Color
vec3 ka = vec3(0.1, 0.1, 0.1);
vec3 ks = vec3(0.8, 0.8, 0.8);

uniform vec3 cameraPos;
varying vec4 fragPos;
varying vec3 N;

void main(void)
{
    vec3 L = normalize(lightPos - vec3(fragPos));
    vec3 V = normalize(cameraPos - vec3(fragPos));

    vec3 H = normalize(L + V);
    float NdotL = dot(N, L);
    float NdotH = dot(N, H);

    vec3 diffuseColor = I * kd * max(0, NdotL);
    vec3 ambientColor = Iamb * ka;
    vec3 specularColor = I * ks * pow(max(0, NdotH), 20);

    // Set the output color to the sum of diffuse, ambient, and specular
    gl_FragColor = vec4(diffuseColor + ambientColor + specularColor, 1.0);
    //gl_FragDepth = 0.3;
}
