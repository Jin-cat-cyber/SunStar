#include "Model.h"

Model::Model(string const& path, bool gamma) : gammaCorrection(gamma)
{
	loadMode(path);
}

void Model::Draw(Shader& shader)
{
	for (unsigned int i = 0; i < meshes.size(); i++)
		meshes[i].Draw(shader);
}

void Model::loadMode(string const& path)
{
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(
		path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace
	);
	/*
	aiProcess_Triangulate
	作用：把所有的模型面片都转换成三角形。因为大多数渲染管线（包括 OpenGL）只支持三角形，所以这是常用的预处理步骤。

	aiProcess_FlipUVs
	作用：翻转所有的纹理坐标的 Y 分量。因为有些模型的 UV 坐标原点在左上，有些在左下，OpenGL 默认原点在左下，所以需要翻转。

	aiProcess_CalcTangentSpace
	作用：计算每个顶点的切线和副切线（tangent/bitangent）。这在做法线贴图（normal mapping）时非常重要，因为需要切线空间。
	这些参数是 Assimp（模型导入库）在导入模型时的处理选项，用于保证导入后的数据适合你的渲染需求。
	*/

	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		std::cerr << "ERROR::ASSIMP::" << importer.GetErrorString() << std::endl;
		return;
	}
	// 获取目录路径
	directory = path.substr(0, path.find_last_of('/'));
	/*	path 是模型文件的完整路径，比如 "res/model/backpack/backpack.obj"。
	find_last_of('/') 找到路径中最后一个 '/' 的位置（即文件名和目录的分界点）。
	substr(0, ...) 从开头截取到最后一个 '/' 之前的所有内容，也就是目录路径。
	结果：directory 变成 "res/model/backpack"。
	*/
	// 递归处理节点
	processNode(scene->mRootNode, scene);
}

void Model::processNode(aiNode* node, const aiScene* scene)
{
	// 处理节点所有网格
	for (unsigned int i = 0; i < node->mNumMeshes; i++)
	{
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		meshes.push_back(processMesh(mesh, scene));
	}
	// 递归处理子节点
	for(unsigned int i = 0; i < node->mNumChildren; i++)
	{
		processNode(node->mChildren[i], scene);
	}
}

Mesh Model::processMesh(aiMesh* mesh, const aiScene* scene)
{
	// 数据
	vector<Vertex>       vertices;
	vector<unsigned int> indices;
	vector<Texture>      textures;

	// 遍历所有顶点
	for (unsigned int i = 0; i < mesh->mNumVertices; i++)
	{
		Vertex vertex;
		glm::vec3 vector;	// 临时变量
		// 位置
		vector.x = mesh->mVertices[i].x;
		vector.y = mesh->mVertices[i].y;
		vector.z = mesh->mVertices[i].z;
		vertex.Position = vector;
		// 法线
		if (mesh->HasNormals())
		{
			vector.x = mesh->mNormals[i].x;
			vector.y = mesh->mNormals[i].y;
			vector.z = mesh->mNormals[i].z;
			vertex.Normal = vector;
		}
		// 纹理坐标
		if (mesh->mTextureCoords[0])
		{
			// 一个顶点最多可包含 8 个不同的纹理坐标。
			// 因此，我们做出如下假设：我们不会使用顶点可能包含多个纹理坐标的模型，所以始终采用第一组纹理坐标（序号 0）
			glm::vec2 vec;
			vec.x = mesh->mTextureCoords[0][i].x;
			vec.y = mesh->mTextureCoords[0][i].y;
			vertex.TexCoords = vec;
			// 切线
			vector.x = mesh->mTangents[i].x;
			vector.y = mesh->mTangents[i].y;
			vector.z = mesh->mTangents[i].z;
			vertex.Tangent = vector;
			// 副切线
			vector.x = mesh->mBitangents[i].x;
			vector.y = mesh->mBitangents[i].y;
			vector.z = mesh->mBitangents[i].z;
			vertex.Bitangent = vector;
		}
		else
			vertex.TexCoords = glm::vec2(0.0f, 0.0f);

		vertices.push_back(vertex);
	}

	// 遍历所有面
	for (unsigned int i = 0; i < mesh->mNumFaces; i++)
	{
		aiFace face = mesh->mFaces[i];

		for (unsigned int j = 0; j < face.mNumIndices; j++)
			indices.push_back(face.mIndices[j]);
	}
	// 处理材质
	aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
	// 我们对着色器中的采样器名称采用一套约定。每个漫反射纹理都应命名为
	// “texture_diffuseN”，其中 N 是从 1 到 MAX_SAMPLER_NUMBER（最大采样器数量）的连续序号。
	// 其他类型的纹理也遵循相同规则，具体如下列表所示：
	// 漫反射纹理：texture_diffuseN
	// 高光纹理：texture_specularN
	// 法线纹理：texture_normalN

	// 1. 漫反射贴图
	vector<Texture> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
	textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end()); // 插入
	// 2. specular maps 高光贴图
	vector<Texture> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
	textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
	// 3. normal maps 法线贴图
	std::vector<Texture> normalMaps = loadMaterialTextures(material, aiTextureType_HEIGHT, "texture_normal");
	textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());
	// 4. height maps 高度贴图
	std::vector<Texture> heightMaps = loadMaterialTextures(material, aiTextureType_AMBIENT, "texture_height");
	textures.insert(textures.end(), heightMaps.begin(), heightMaps.end());

	return Mesh(vertices, indices, textures);
}

vector<Texture> Model::loadMaterialTextures(aiMaterial* mat, aiTextureType type, string typeName)
{
	vector<Texture> textures;
	for (unsigned int i = 0; i < mat->GetTextureCount(type); i++)
	{
		aiString str;
		mat->GetTexture(type, i, &str);
		// 检查纹理是否重复
		bool skip = false;
		for (unsigned int j = 0; j < textures_loaded.size(); j++)
		{
			if (std::strcmp(textures_loaded[j].path.data(), str.C_Str()) == 0)
			{
				textures.push_back(textures_loaded[j]);
				skip = true; // 跳过
				break;
			}
		}
		if (!skip)
		{
			Texture texture;
			texture.id = TextureFromFile(str.C_Str(), directory);
			texture.type = typeName + std::to_string(i + 1);
			texture.path = str.C_Str();
			textures.push_back(texture);
			textures_loaded.push_back(texture); // 记录已加载的纹理
		}
	}
	return textures;
}

