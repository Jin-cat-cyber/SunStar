#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>


#include "Mesh.h"
#include "Shader.h"

#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <vector>
#include <string>

using namespace std;

unsigned int TextureFromFile(const char* path, const string & directory, bool gamma = false);
/*
1.	const char* path
含义：纹理图片的文件名或相对路径（不包含目录）。
用途：指定要加载的图片文件名。
2.	const string& directory
含义：图片所在的目录路径。
用途：与 path 组合，得到图片的完整路径（通常是 directory + '/' + path）。
3.	bool gamma
含义：是否使用 gamma 校正（默认值为 false）。
用途：决定加载纹理时是否进行 gamma 校正处理，影响渲染效果。

示例调用：
unsigned int texID = TextureFromFile("diffuse.png", "assets/textures", true);
这里 "diffuse.png" 是文件名，"assets/textures" 是目录，true 表示需要 gamma 校正。
*/
// 三个参数分别为

class Model
{
public:
	// 模型数据
	vector<Texture> textures_loaded;
	vector<Mesh>	meshes;
	string directory;
	bool gammaCorrection;

	// 构造函数, 传入模型文件路径
	Model(string const& path, bool gamma = false);
	// 绘制模型, 遍历所有网格并绘制

	void Draw(Shader& shader);

private:
	// 从文件中加载模型
	void loadMode(string const& path);
	// 递归处理节点
	void processNode(aiNode* node, const aiScene* scene);
	Mesh processMesh(aiMesh* mesh, const aiScene* scene);
	// 检查纹理是否重复, 若没有则加载
	vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, string typeName);
};

