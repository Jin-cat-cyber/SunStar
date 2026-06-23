#version 330 core
in vec2 TexCoords;

out vec4 FragColor;

uniform sampler2D hdrImage;
uniform float threshold = 1.0;	// HDR 亮度阈值，调节哪些区域产生 Bloom

void main()
{
	vec3 color = texture(hdrImage, TexCoords).rgb;
	float brightness = dot(color, vec3(0.2126, 0.7152, 0.0722));
	if (brightness > threshold)
		FragColor = vec4(color, 1.0);
	else
		FragColor = vec4(0.0, 0.0, 0.0, 1.0);
}