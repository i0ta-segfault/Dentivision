#version 410

layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 colors;
layout(location = 2) in vec2 aTexCoord;
out vec2 TexCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform float time;

out vec3 fragCoord;
out vec3 customColors;

float degtorad(float degrees){
    return degrees * 3.14159265359 / 180.0;
}

void main() {
    vec3 newpos = pos;
    fragCoord = newpos;
    customColors = colors;
    gl_Position = projection * view * model * vec4(newpos, 1.0);
    TexCoord = aTexCoord;
}