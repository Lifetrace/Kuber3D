#version 460

layout(lines) in;
layout(line_strip, max_vertices = 2) out;

in vec4 ourColor[];
in vec4 vClip[];

out vec4 gColor;
flat out vec2 gA;
flat out vec2 gB;

uniform vec2 uViewport;

vec2 clipToScreen(vec4 clipPos){
    vec2 ndc = clipPos.xy / clipPos.w;
    vec2 uv  = ndc * 0.5 + 0.5;
    return uv * uViewport; 
}

void main(){
    gA = clipToScreen(vClip[0]);
    gB = clipToScreen(vClip[1]);

    gl_Position = gl_in[0].gl_Position;
    gColor = ourColor[0];
    EmitVertex();

    gl_Position = gl_in[1].gl_Position;
    gColor = ourColor[1];
    EmitVertex();

    EndPrimitive();
}