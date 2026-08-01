#version 330 core
in vec2 TexCoords;

out vec4 FragColor;

uniform sampler2D flareTexture;
uniform vec4 flareColor = vec4(1.0);

void main()
{
	vec4 texColor = texture(flareTexture, TexCoords);
	vec4 color = texture(flareTexture, TexCoords);

	FragColor = texColor * flareColor;
}

// BUG°æ±¾
// #version 330 core
// in vec2 TexCoords;

// out vec4 FragColor;

// uniform sampler2D flareTexture;
// uniform sampler2D flareTexture1;
// uniform vec4 flareColor = vec4(1.0);

// void main()
// {
// 	vec4 texColor = texture(flareTexture, TexCoords);
// 	vec4 tex2Color = texture(flareTexture1, TexCoords);
// 	vec4 color = texColor + tex2Color;

// 	FragColor = color * flareColor;
// }