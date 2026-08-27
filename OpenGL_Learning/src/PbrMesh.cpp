#include "PbrMesh.h"

PbrMesh::PbrMesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures)
{
	this->vertices = vertices;
	this->indices = indices;
	this->textures = textures;

	setupMesh();
}

void PbrMesh::Draw(Shader& shader)
{
	for (unsigned int i = 0; i < textures.size(); i++)
	{
		std::string type = textures[i].type;
		const char* uniform = nullptr;
		int unit = -1;

		if (type.rfind("albedo", 0) == 0) { uniform = "albedoMap";    unit = 3; }
		else if (type.rfind("normal", 0) == 0) { uniform = "normalMap";    unit = 4; }
		else if (type.rfind("metalness", 0) == 0) { uniform = "metallicMap";  unit = 5; }
		else if (type.rfind("roughness", 0) == 0) { uniform = "roughnessMap"; unit = 6; }
		else if (type.rfind("emission", 0) == 0) { uniform = "emissionMap";  unit = 8; }
		else if (type.rfind("ao", 0) == 0) { uniform = "aoMap";        unit = 7; }
		else
		{
			// 老约定：type 本身就是 uniform 名（texture_diffuse1 等），保持原行为
			glActiveTexture(GL_TEXTURE0 + i);
			glBindTexture(GL_TEXTURE_2D, textures[i].id);
			glUniform1i(glGetUniformLocation(shader.ID, type.c_str()), i);
			continue;
		}

		glActiveTexture(GL_TEXTURE0 + unit);
		glBindTexture(GL_TEXTURE_2D, textures[i].id);
		glUniform1i(glGetUniformLocation(shader.ID, uniform), unit);
	}

	glBindVertexArray(VAO);
	glDrawElements(GL_TRIANGLES, static_cast<unsigned int>(indices.size()), GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
	glActiveTexture(GL_TEXTURE0);
}



void PbrMesh::setupMesh()
{
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	// 顶点数据
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	// 索引数据
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);
	// 位置属性
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
	// 法线属性
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(float) * 3));
	// 纹理坐标属性
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(float) * 6));
	// 切线属性
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(float) * 8));
	// 副切线属性
	glEnableVertexAttribArray(4);
	glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(float) * 11));

	// 骨骼ID属性
	glEnableVertexAttribArray(5);
	glVertexAttribIPointer(5, 4, GL_INT, sizeof(Vertex), (void*)(sizeof(float) * 14)); // 注意这里是 glVertexAttribIPointer
	// 骨骼权重属性
	glEnableVertexAttribArray(6);
	glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(float) * 18));

	glBindVertexArray(0);
}