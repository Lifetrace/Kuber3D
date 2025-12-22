#version 460 core
in vec3 ourColor;
out vec4 FragColor;

uniform int uIsPoint; 

void main()
{
    float alpha = 1.0;

    if (uIsPoint == 1)
    {
        vec2 uv = gl_PointCoord * 2.0 - 1.0;
        float r = length(uv);

        float radius = 1.0;
        float aa = fwidth(r);

        alpha = 1.0 - smoothstep(radius - aa, radius + aa, r);
        if (alpha <= 0.0) discard;
    }

    FragColor = vec4(ourColor, alpha);
}
