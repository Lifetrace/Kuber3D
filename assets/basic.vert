#version 460
layout(location = 0) in vec3 vPos;
layout(location = 1) in vec4 vColor;

out vec4 ourColor;

uniform mat4 uView;
uniform mat4 uProj;
uniform mat4 uModel;

void main(){
    gl_Position = uProj * uView * uModel * vec4(vPos, 1.0);
    gl_PointSize = 6.0;
    ourColor = vColor;
}
