#version 460
in vec4 ourColor;
out vec4 FragColor;

uniform int uIsPoints; 

void main(){
    if (uIsPoints == 1) {
        vec2 p = gl_PointCoord * 2.0 - 1.0;
        if (dot(p,p) > 1.0) discard;
    }
    FragColor = ourColor;
}

