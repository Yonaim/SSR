#ifndef QUAD_HPP
#define QUAD_HPP

#include <glad/glad.h>

class Quad
{
private:
	GLuint VAO, VBO, EBO;
public:
	Quad(void);
	~Quad();

	GLuint GetVAO() {
		return VAO;
	}
};

Quad::Quad(void)
{
	float vertices[] = {
		// position			 // UV (texture coordinates) ???? 
		-1.0f,  1.0f, -1.0f,  0.0f, 1.0f,
		1.0f,  1.0f, -1.0f,  1.0f, 1.0f,
		-1.0f, -1.0f, -1.0f,  0.0f, 0.0f,
		1.0f, -1.0f, -1.0f,  1.0f, 0.0f
	};
	unsigned int indices[] = {
		0, 1, 2,
		1, 2, 3
	};

	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	// VAO
	glBindVertexArray(VAO);
	// VBO
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	// EBO
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
	
	// VBO로부터 온 속성 포인터를 VAO에 넣어줌
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 5, (void*)0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 5, (void*)(sizeof(float) * 3));
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	
	glBindVertexArray(0);
}

Quad::~Quad()
{
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &EBO);
}

#endif
