#include "Debug_Shader.h"

// 老版本
Debug_Shader::Debug_Shader(const char* vertexPath, const char* fragmentPath)
{
	// 1. 从文件路径中获取顶点/片段着色器
	std::string vertexCode;
	std::string fragmentCode;
	std::ifstream vShaderFile;
	std::ifstream fShaderFile;
	// 保证ifstream对象可以抛出异常
	vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	try
	{
		// 打开文件
		vShaderFile.open(vertexPath);
		fShaderFile.open(fragmentPath);
		std::stringstream vShaderStream, fShaderStream;
		// 读取文件的缓冲内容到流中
		vShaderStream << vShaderFile.rdbuf();
		fShaderStream << fShaderFile.rdbuf();
		// 关闭文件处理器
		vShaderFile.close();
		fShaderFile.close();
		// 转换数据流到string
		vertexCode = vShaderStream.str();
		fragmentCode = fShaderStream.str();
	}
	catch (std::ifstream::failure e)
	{
		std::cout << "ERROR::SHADER::FILE_NOT_SUCCESSFULLY_READ" << std::endl;
	}
	const char* vShaderCode = vertexCode.c_str();
	const char* fShaderCode = fragmentCode.c_str();
	// 2. 编译顶点着色器
	unsigned int vertex, fragment;
	int success;
	char infoLog[512];

	// 顶点着色器
	vertex = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex, 1, &vShaderCode, NULL);
	glCompileShader(vertex);
	checkCompileErrors(vertex, "VERTEX");
	// 打印编译错误（如果有的话）
	glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(vertex, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
	}

	// 片段着色器
	fragment = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment, 1, &fShaderCode, NULL);
	glCompileShader(fragment);
	checkCompileErrors(fragment, "FRAGMENT");
	// 打印编译错误（如果有的话）
	glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(fragment, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
	}

	// 着色器程序
	ID = glCreateProgram(); // 创建程序对象
	glAttachShader(ID, vertex); // 将顶点着色器附加到程序对象
	glAttachShader(ID, fragment); // 将片段着色器附加到程序对象
	glLinkProgram(ID); // 链接着色器程序
	// 打印链接错误（如果有的话）
	glGetProgramiv(ID, GL_LINK_STATUS, &success);
	if (!success)
	{
		glGetProgramInfoLog(ID, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
	}

	// 删除着色器，他们已经被链接到程序对象中，已经不需要了
	glDeleteShader(vertex);
	glDeleteShader(fragment);
}

// 构造函数重载 —— 更新的新构造函数，包含几何着色器
Debug_Shader::Debug_Shader(const char* vertexPath, const char* fragmentPath, const char* geometryPath = NULL)
{
	// 1. 从文件路径中获取顶点/片段着色器
	std::string vertexCode;
	std::string fragmentCode;
	std::string geometryCode;
	std::ifstream vShaderFile;
	std::ifstream fShaderFile;
	std::ifstream gShaderFile;
	// 保证ifstream对象可以抛出异常
	vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	gShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	try
	{
		// 打开文件
		vShaderFile.open(vertexPath);
		fShaderFile.open(fragmentPath);
		std::stringstream vShaderStream, fShaderStream;
		// 读取文件到数据流中
		vShaderStream << vShaderFile.rdbuf();
		fShaderStream << fShaderFile.rdbuf();
		// 关闭数据处理器
		vShaderFile.close();
		fShaderFile.close();
		// 转换数据流到string
		vertexCode = vShaderStream.str();
		fragmentCode = fShaderStream.str();
		// 如果几何着色器存在，则也加载几何着色器
		if (geometryPath != nullptr)
		{
			gShaderFile.open(geometryPath);
			std::stringstream gShaderStream;
			gShaderStream << gShaderFile.rdbuf();
			gShaderFile.close();
			geometryCode = gShaderStream.str();
		}
	}
	catch (std::ifstream::failure& e)
	{
		std::cout << "ERROR::SHADER::FILE_NOT_SUCCESSFULLY_READ: " << e.what() << std::endl;
	}
	const char* vShaderCode = vertexCode.c_str();
	const char* fShaderCode = fragmentCode.c_str();
	// 2. 编译着色器
	unsigned int vertex, fragment;
	// 顶点着色器
	vertex = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex, 1, &vShaderCode, NULL);
	glCompileShader(vertex);
	checkCompileErrors(vertex, "VERTEX");
	// 片段着色器
	fragment = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment, 1, &fShaderCode, NULL);
	glCompileShader(fragment);
	checkCompileErrors(fragment, "FRAGMENT");
	// 如果几何着色器存在，也编译几何着色器
	unsigned int geometry;
	if (geometryPath != nullptr)
	{
		const char* gShaderCode = geometryCode.c_str();
		geometry = glCreateShader(GL_GEOMETRY_SHADER);
		glShaderSource(geometry, 1, &gShaderCode, NULL);
		glCompileShader(geometry);
		checkCompileErrors(geometry, "GEOMETRY");
	}
	// 着色器程序
	ID = glCreateProgram();
	glAttachShader(ID, vertex);
	glAttachShader(ID, fragment);
	if (geometryPath != nullptr)
		glAttachShader(ID, geometry);
	glLinkProgram(ID);
	checkCompileErrors(ID, "PROGRAM");
	// 删除着色器，他们已经被链接到程序对象中，已经不需要了
	glDeleteShader(vertex);
	glDeleteShader(fragment);
	if (geometryPath != nullptr)
		glDeleteShader(geometry);
}




// 使用/激活程序
void Debug_Shader::use()
{
	glUseProgram(ID);
}

// uniform工具函数
void Debug_Shader::setBool(const std::string& name, bool value) const
{
	int loc = getUniformLocation(name);
	glUniform1i(loc, (int)value);
}

void Debug_Shader::setInt(const std::string& name, int value) const
{
	int loc = getUniformLocation(name);
	glUniform1i(loc, value);
}

void Debug_Shader::setFloat(const std::string& name, float value) const
{
	int loc = getUniformLocation(name);
	glUniform1f(loc, value);
}

void Debug_Shader::setVec2(const std::string& name, const glm::vec2& value) const
{
	int loc = getUniformLocation(name);
	glUniform2fv(loc, 1, &value[0]);
}
void Debug_Shader::setVec2(const std::string& name, float x, float y) const
{
	int loc = getUniformLocation(name);
	glUniform2f(loc, x, y);
}
// ------------------------------------------------------------------------
void Debug_Shader::setVec3(const std::string& name, const glm::vec3& value) const
{
	int loc = getUniformLocation(name);
	glUniform3fv(loc, 1, &value[0]);
}
void Debug_Shader::setVec3(const std::string& name, float x, float y, float z) const
{
	int loc = getUniformLocation(name);
	glUniform3f(loc, x, y, z);
}
// ------------------------------------------------------------------------
void Debug_Shader::setVec4(const std::string& name, const glm::vec4& value) const
{
	int loc = getUniformLocation(name);
	glUniform4fv(loc, 1, &value[0]);
}
void Debug_Shader::setVec4(const std::string& name, float x, float y, float z, float w) const
{
	int loc = getUniformLocation(name);
	glUniform4f(loc, x, y, z, w);
}
// ------------------------------------------------------------------------
void Debug_Shader::setMat2(const std::string& name, const glm::mat2& mat) const
{
	int loc = getUniformLocation(name);
	glUniformMatrix2fv(loc, 1, GL_FALSE, &mat[0][0]);
}
// ------------------------------------------------------------------------
void Debug_Shader::setMat3(const std::string& name, const glm::mat3& mat) const
{
	int loc = getUniformLocation(name);
	glUniformMatrix3fv(loc, 1, GL_FALSE, &mat[0][0]);
}
// ------------------------------------------------------------------------
void Debug_Shader::setMat4(const std::string& name, const glm::mat4& mat) const
{
	int loc = getUniformLocation(name);
	glUniformMatrix4fv(loc, 1, GL_FALSE, &mat[0][0]);
}

// 检查编译错误
void Debug_Shader::checkCompileErrors(GLuint shader, std::string type)
{
	GLint success;
	GLchar infoLog[1024];
	if (type != "PROGRAM")
	{
		glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			glGetShaderInfoLog(shader, 1024, NULL, infoLog);
			std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
		}
	}
	else
	{
		glGetProgramiv(shader, GL_LINK_STATUS, &success);
		if (!success)
		{
			glGetProgramInfoLog(shader, 1024, NULL, infoLog);
			std::cout << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
		}
	}
}

// 检查Uniform绑定状态
int Debug_Shader::getUniformLocation(const std::string& name) const
{
	int loc = glGetUniformLocation(ID, name.c_str());
	if (loc == -1)
	{
		std::cerr << "WARNING: Uniform '" << name << "' not found in shader " << ID << std::endl;
	}
	return loc;
}

