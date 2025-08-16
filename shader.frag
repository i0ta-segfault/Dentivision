#version 410

in vec3 fragCoord;
in vec3 customColors;
flat in int vertexType;
out vec4 fragColor;

uniform vec3 camPos;
uniform float makeTeethYellow;
uniform float dentalCalculus;

vec3 gumsCoord = vec3(0.0);

float noise(vec3 p) {
    return fract(sin(dot(p, vec3(12.9898, 78.233, 37.719))) * 43758.5453);
}

vec3 applyCalculusToGums(vec3 baseColor, vec3 fragCoord, float intensity) {
    float noiseValue = noise(fragCoord * 0.5 + camPos * 0.01) * intensity;
    vec3 calculusColor = vec3(0.8, 0.7, 0.3);
    return mix(baseColor, calculusColor, noiseValue);
}

void main() {
    vec3 objectColor = customColors;
    vec3 result = objectColor;

    if (vertexType == 2) {  // Gums
        gumsCoord = fragCoord; 
        float gumNoise = noise(fragCoord * 0.1) * makeTeethYellow;
        vec3 healthyPink = vec3(1.0, 0.75, 0.8);  
        vec3 decayedYellowBrown = vec3(0.8, 0.65, 0.3);
        vec3 yellowedGums = mix(healthyPink, decayedYellowBrown, gumNoise);

        float calculusNoise = noise(fragCoord * 0.5 + camPos * 0.01) * dentalCalculus;
        vec3 calculusColor = vec3(0.8, 0.7, 0.3);
        vec3 gumsWithCalculus = mix(yellowedGums, calculusColor, calculusNoise);

        result = gumsWithCalculus;
    } 
    else if (vertexType == 1) {  // Teeth
        float yellowingNoise = noise(fragCoord * 0.1) * makeTeethYellow;
        vec3 yellow = vec3(1.0, 1.0, 0.0);
        result = mix(objectColor, yellow, yellowingNoise);
    }

    fragColor = vec4(result, 0.7);
}
