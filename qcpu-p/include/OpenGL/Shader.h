//
//	Shader
//

#pragma once

#include <glad/glad.h>

#include <map>
#include <string>

using ShaderProgram = GLuint;

class Shader
{
	using UniformMap = std::map<std::string, unsigned int>;
	using AttributeMap = std::map<std::string, unsigned int>;

public:

	static ShaderProgram InvalidProgram;
	static Shader* DefaultShader;

	Shader();
	Shader(const std::string& vertexPath, const std::string& fragmentPath);
	~Shader();

	void Use() const;
	void Create(const std::string& vertexPath, const std::string& fragmentPath);

	void SetBool(const std::string& name, bool value) const;
	void SetInt(const std::string& name, int value) const;
	void SetFloat(const std::string& name, float value) const;
	void SetVector2(const std::string& name, float x, float y) const;
	void SetVector3(const std::string& name, float x, float y, float z) const;
	void SetVector4(const std::string& name, float x, float y, float z, float w) const;

	int GetUniformLocation(const std::string& name) const;

	GLuint GetProgram() const;

private:

	std::string name;
	UniformMap uniforms;
	AttributeMap attributes;
	ShaderProgram program;
	bool validate;

private:

	std::string ReadFileAsString(const std::string& filepath);
	GLuint CompileShader(GLenum type, const std::string& source);
};
