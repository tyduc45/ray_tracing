#version 460

in vec2 vUV;
out vec4 FragColor;

uniform sampler2D uRayTex;

void main()
{
    FragColor = texture(uRayTex, vUV);
}
