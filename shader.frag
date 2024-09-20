#version 410

in vec3 fragCoord;
in vec3 customColors;
in vec2 TexCoord;
out vec4 fragColor;

uniform vec3 camPos;
uniform sampler2D texture1;

float degtorad(float degrees){
    return degrees * 3.14159265359 / 180.0;
}

void main(){
    vec3 objectColor = customColors;
    vec3 result = objectColor;
    vec4 texColor = texture(texture1, TexCoord);

    fragColor = texColor * vec4(result, .7);
}
