#version 460

in vec4 gColor;
flat in vec2 gA;
flat in vec2 gB;

out vec4 FragColor;

uniform int   uDashed; 
uniform float uDashPx; 
uniform float uGapPx;

void main(){
    if (uDashed == 1){
        vec2 p = gl_FragCoord.xy;

        vec2 ab = gB - gA;
        float len = length(ab);

        if (len > 0.0001){
            vec2 dir = ab / len;
            float t = dot(p - gA, dir);
            t = clamp(t, 0.0, len);

            float period = max(0.0001, uDashPx + uGapPx);
            float m = mod(t, period);

            if (m > uDashPx) discard;
        }
    }

    FragColor = gColor;
}
