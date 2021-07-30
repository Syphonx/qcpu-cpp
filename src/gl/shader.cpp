//
//	Shader
//

#include "gl/shader.h"

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <assert.h>
#include <vector>

#include <glad/glad.h>
#include <stdio.h>

namespace ShaderUtil
{
	void ValidateShader(GLuint shader, const std::string& type)
	{
		GLint					success;
		std::vector<GLchar>		infoLog;
		GLint					maxLength = 0;

		if (type != "Program")
		{
			glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
			if (!success)
			{
				glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);
				infoLog.resize(maxLength);

				glGetShaderInfoLog(shader, maxLength, &maxLength, &infoLog[0]);
				printf("Failed to compile shader %s (%s)\n", type.c_str(), &infoLog[0]);
			}
		}
		else
		{
			glGetProgramiv(shader, GL_LINK_STATUS, &success);
			if (!success)
			{
				glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);
				infoLog.resize(maxLength);

				glGetProgramInfoLog(shader, maxLength, &maxLength, &infoLog[0]);
				printf("Failed to link shader %s (%s)\n", type.c_str(), &infoLog[0]);
			}
		}
	}
}

ShaderProgram	Shader::InvalidProgram = -1;
Shader*			Shader::DefaultShader;

Shader::Shader()
{
	program = Shader::InvalidProgram;
	validate = false;
}

Shader::Shader(const std::string& vertexPath, const std::string& fragmentPath)
{
	program = Shader::InvalidProgram;
	validate = false;
	Create(vertexPath, fragmentPath);
}

Shader::~Shader()
{

}

void Shader::Use() const
{
	glUseProgram(program);
}

void Shader::Create(const std::string& vertexPath, const std::string& fragmentPath)
{
	std::string vertexCode = ReadFileAsString(vertexPath);
	std::string fragmentCode = ReadFileAsString(fragmentPath);
	unsigned int vertex, fragment;

	// Compile the vertex Shader
	vertex = CompileShader(GL_VERTEX_SHADER, vertexCode);
	ShaderUtil::ValidateShader(vertex, "Vertex");

	// Compile the fragment Shader
	fragment = CompileShader(GL_FRAGMENT_SHADER, fragmentCode);
	ShaderUtil::ValidateShader(fragment, "Fragment");

	// Create the shader Program
	program = glCreateProgram();
	glAttachShader(program, vertex);
	glAttachShader(program, fragment);
	glLinkProgram(program);
	ShaderUtil::ValidateShader(program, "Program");

	// Query the number of active uniforms and attributes
	int nrAttributes, nrUniforms;
	glGetProgramiv(program, GL_ACTIVE_ATTRIBUTES, &nrAttributes);
	glGetProgramiv(program, GL_ACTIVE_UNIFORMS, &nrUniforms);

	// Iterate over all active attributes
	int size;
	char buffer[128];
	for (int i = 0; i < nrAttributes; ++i)
	{
		GLenum glType;
		glGetActiveAttrib(program, i, sizeof(buffer), 0, &size, &glType, buffer);
		attributes[std::string(buffer)] = glGetAttribLocation(program, buffer);
	}

	// Iterate over all active uniforms
	for (int i = 0; i < nrUniforms; ++i)
	{
		GLenum glType;
		glGetActiveUniform(program, i, sizeof(buffer), 0, &size, &glType, buffer);
		uniforms[std::string(buffer)] = glGetUniformLocation(program, buffer);
	}

	// Delete the shaders as they're linked into our program now and no longer necessery
	glDetachShader(program, vertex);
	glDetachShader(program, fragment);
	glDeleteShader(vertex);
	glDeleteShader(fragment);
}

void Shader::SetBool(const std::string& name, bool value) const
{
	GLint location = GetUniformLocation(name);
	if (location == -1)
	{
		if (validate)
		{
			printf("Uniform location not found for (%s)\n", name.c_str());
		}
		return;
	}
	glUniform1i(location, (int)value);
}

void Shader::SetInt(const std::string& name, int value) const
{
	GLint location = GetUniformLocation(name);
	if (location == -1)
	{
		if (validate)
		{\
			printf("Uniform location not found for (%s)\n", name.c_str());
		}
		return;
	}
	glUniform1i(location, value);
}

void Shader::SetFloat(const std::string& name, float value) const
{
	GLint location = GetUniformLocation(name);
	if (location == -1)
	{
		if (validate)
		{
			printf("Uniform location not found for (%s)\n", name.c_str());
		}
		return;
	}
	glUniform1f(location, value);
}

void Shader::SetVector2(const std::string& name, float x, float y) const
{
	GLint location = GetUniformLocation(name);
	if (location == -1)
	{
		if (validate)
		{
			printf("Uniform location not found for (%s)\n", name.c_str());
		}
		return;
	}
	glUniform2f(location, x, y);
}

void Shader::SetVector3(const std::string& name, float x, float y, float z) const
{
	GLint location = GetUniformLocation(name);
	if (location == -1)
	{
		if (validate)
		{
			printf("Uniform location not found for (%s)\n", name.c_str());
		}
		return;
	}
	glUniform3f(location, x, y, z);
}

void Shader::SetVector4(const std::string& name, float x, float y, float z, float w) const
{
	GLint location = GetUniformLocation(name);
	if (location == -1)
	{
		if (validate)
		{
			printf("Uniform location not found for (%s)\n", name.c_str());
		}
		return;
	}
	glUniform4f(location, x, y, z, w);
}

int Shader::GetUniformLocation(const std::string& name) const
{
	if (uniforms.find(name) != uniforms.end())
	{
		return uniforms.at(name);
	}
	return -1;
}

GLuint Shader::GetProgram() const
{
	return program;
}

std::string Shader::ReadFileAsString(const std::string& filepath)
{
	std::string result;

	if (std::ifstream in = std::ifstream(filepath, std::ios::in | std::ios::binary))
	{
		in.seekg(0, std::ios::end);
		result.resize((size_t)in.tellg());
		in.seekg(0, std::ios::beg);
		in.read(&result[0], result.size());
		in.close();
	}
	else
	{
		printf("Could not open file '%s'\n", filepath.c_str());
	}

	return result;
}

GLuint Shader::CompileShader(GLenum type, const std::string& source)
{
	GLuint shader = glCreateShader(type);
	const GLchar* sourceString = source.c_str();
	glShaderSource(shader, 1, &sourceString, NULL);
	glCompileShader(shader);

	return shader;
}
