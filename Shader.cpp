#include "GL/glew.h"
#include "Shader.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

std::string Shader::readFile(const std::string& path) {
	std::ifstream f(path);
	std::stringstream ss;
	ss << f.rdbuf();
	return ss.str();
}

GLuint Shader::compile(GLenum type, const std::string& src) {
	GLuint s = glCreateShader(type);
	const char* p = src.c_str();
	glShaderSource(s, 1, &p, NULL);
	glCompileShader(s);

	int ok;
	glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
	if (!ok) {
		char log[1024];
		glGetShaderInfoLog(s, 1024, NULL, log);
		std::cerr << "Shader compile error:\n" << log << std::endl;
	}

	return s;
}

Shader::Shader(const std::string& vertexPath, const std::string& fragmentPath) {
	std::string vs = readFile(vertexPath);
	std::string fs = readFile(fragmentPath);

	GLuint v = compile(GL_VERTEX_SHADER, vs);
	GLuint f = compile(GL_FRAGMENT_SHADER, fs);

	program = glCreateProgram();
	glAttachShader(program, v);
	glAttachShader(program, f);
	glLinkProgram(program);

	int ok;
	glGetProgramiv(program, GL_LINK_STATUS, &ok);
	if (!ok) {
		char log[1024];
		glGetProgramInfoLog(program, 1024, NULL, log);
		std::cerr << "Shader link error:\n" << log << std::endl;
	}

	glDeleteShader(v);
	glDeleteShader(f);
}

Shader::~Shader() {
	glDeleteProgram(program);
}

void Shader::use() const {
	glUseProgram(program);
}

GLuint Shader::id() const {
	return program;
}

void Shader::setInt(const std::string& name, int v) const {
	glUniform1i(glGetUniformLocation(program, name.c_str()), v);
}

void Shader::setFloat(const std::string& name, float v) const {
	glUniform1f(glGetUniformLocation(program, name.c_str()), v);
}

void Shader::setVec2(const std::string& name, float x, float y) const {
	glUniform2f(glGetUniformLocation(program, name.c_str()), x, y);
}

void Shader::setVec3(const std::string& name, float x, float y, float z) const {
	glUniform3f(glGetUniformLocation(program, name.c_str()), x, y, z);
}

void Shader::setMat4(const std::string& name, const float* mat) const {
	glUniformMatrix4fv(glGetUniformLocation(program, name.c_str()), 1, GL_FALSE, mat);
}

void Shader::setBool(const std::string& name, bool value) const {
	glUniform1i(glGetUniformLocation(program, name.c_str()), value);
}