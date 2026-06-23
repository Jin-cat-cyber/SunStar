#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <glfw3.h>
#include "Shader.h"

#include <string>
#include <vector>

#define MAX_BONE_INFLUENCE 4

struct Vertex
{
	// 位置
	glm::vec3 Position;
	// 法线
	glm::vec3 Normal;
	// 纹理坐标
	glm::vec2 TexCoords;
	// 切线
	glm::vec3 Tangent;
	// 副切线
	glm::vec3 Bitangent;
	// 骨骼ID
	int BoneIDs[MAX_BONE_INFLUENCE];
	// 每块骨骼权重
	float Weights[MAX_BONE_INFLUENCE];
};

struct Texture
{
	unsigned int id;
	std::string type;
	std::string path;
};

class Mesh
{
public:
	// 网格数据
	std::vector<Vertex>       vertices; // 顶点
	std::vector<unsigned int> indices;	// 索引
	std::vector<Texture>      textures;	// 纹理
	unsigned int VAO;

	// 构造函数
	Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures);

	// 绘制网格
	void Draw(Shader& shader);

private:
	// 渲染数据
	unsigned int VBO, EBO;
	// 初始化所有缓冲区对象/数组
	void setupMesh();
};

