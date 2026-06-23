// #version 330 core
// out vec4 FragColor;

// in vec3 fColor;

// void main()
// {
//     FragColor = vec4(fColor, 1.0);   
// }


// // VER 2.0  Boom Model
// #version 330 core
// out vec4 FragColor;

// in vec2 TexCoords;

// uniform sampler2D texture_diffuse1;

// void main()
// {
//     FragColor = texture(texture_diffuse1, TexCoords);
// }


// VER 3.0  Normal Model
#version 330 core
out vec4 FragColor;

void main()
{
	FragColor = vec4(1.0, 0.388, 0.278, 1.0);
}
