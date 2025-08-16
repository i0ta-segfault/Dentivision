#version 410

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;
layout(location = 2) in int type;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform float makeTeethYellow;
uniform float shrinkTeeth;
uniform float gingivalRecession;

out vec3 fragCoord;   
out vec3 customColors;  
flat out int vertexType;

float noise(vec3 p) {
    return fract(sin(dot(p, vec3(12.9898, 78.233, 37.719))) * 43758.5453);
}

void main() {
    vec3 adjustedPosition = position;

    if (type == 1) { // Teeth
        float shrinkFactor = 1.0 - (shrinkTeeth * 0.01);
        adjustedPosition *= shrinkFactor;
    } 
    
    if (type == 2) { // Gums
        float shrinkFactor = 1.0 - (gingivalRecession * 0.09);
        adjustedPosition *= shrinkFactor; 
    }

    fragCoord = adjustedPosition;
    customColors = color;        
    vertexType = type;           

    gl_Position = projection * view * model * vec4(adjustedPosition, 1.0);
}
