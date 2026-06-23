// VER 1.0
// #version 330 core
// out vec4 FragColor;

// in vec2 TexCoords;

// uniform sampler2D texture1;

// void main()
// {    
//     FragColor = texture(texture1, TexCoords);
// }

// VER 1.1  深度线性化
// #version 330 core
// out vec4 FragColor;

// float near = 0.1; 
// float far  = 100.0; 
  
// float LinearizeDepth(float depth) 
// {
//     float z = depth * 2.0 - 1.0; // 转换为 NDC
//     return (2.0 * near * far) / (far + near - z * (far - near));	
// }

// void main()
// {             
//     float depth = LinearizeDepth(gl_FragCoord.z) / far; // 为了演示除以 far
//     FragColor = vec4(vec3(depth), 1.0);
// }

//VER 1.2 stencil_test
// #version 330 core
// out vec4 FragColor;

// void main()
// {
//     FragColor = vec4(0.04, 0.28, 0.26, 1.0);
// }

// VER 2.0 blending 添加 alpha通道
// #version 330 core
// out vec4 FragColor;

// in vec2 TexCoords;
// uniform sampler2D texture1;

// void main()
// {
//     vec4 texColor = texture(texture1, TexCoords);
//     if(texColor.a < 0.1)
//         discard;
//     FragColor = texColor;
// }

// VER 2.1  blending 混合 启用Alpha函数
#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D texture1;

void main()
{             
    FragColor = texture(texture1, TexCoords);
}