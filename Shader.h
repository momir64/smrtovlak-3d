#pragma once
#include <GL/glew.h>
#include <string>

class Shader {
	GLuint program = 0;

	static std::string readFile(const std::string& path);
	static GLuint compile(GLenum type, const std::string& src);

public:
	Shader(const std::string& vertexPath, const std::string& fragmentPath);
	~Shader();

	void use() const;
	GLuint id() const;

	void setInt(const std::string& name, int v) const;
	void setFloat(const std::string& name, float v) const;
	void setVec2(const std::string& name, float x, float y) const;
	void setVec3(const std::string& name, float x, float y, float z) const;
	void setMat4(const std::string& name, const float* mat) const;
	void setBool(const std::string& name, bool value) const;
};
