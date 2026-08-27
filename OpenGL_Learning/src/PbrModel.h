#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>


#include "PbrMesh.h"
#include "Shader.h"

#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <vector>
#include <string>

#include <algorithm>   // std::transform
#include <cctype>      // ::tolower

using namespace std;

unsigned int TextureFromFilePBR(const char* path, const string& directory, bool gamma = false);

unsigned int TextureFromMemory(const unsigned char* data, int len, bool gamma = false);

// 三个参数分别为

class PbrModel
{
public:
	// 模型数据
	vector<Texture> textures_loaded;
	vector<PbrMesh>	meshes;
	string directory;
	bool gammaCorrection;

	// 构造函数, 传入模型文件路径
	PbrModel(string const& path, bool gamma = false);
	// 绘制模型, 遍历所有网格并绘制

	void Draw(Shader& shader);

private:
	// 从文件中加载模型
	void loadMode(string const& path);
	// 递归处理节点
	void processNode(aiNode* node, const aiScene* scene);
	PbrMesh processMesh(aiMesh* mesh, const aiScene* scene);
	// 检查纹理是否重复, 若没有则加载
	vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, string typeName, const aiScene* scene);
	//vector<Texture> loadPbrByName(const string& matBase);
};

