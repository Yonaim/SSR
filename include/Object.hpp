#ifndef OBJECT_HPP
#define OBJECT_HPP

#include <glad/glad.h>
#include <vector>

#include "Mesh.hpp"
#include "Shader.hpp"
#include "cyTriMesh.h"

class Object
{
private:
	cyTriMesh mesh; // pointer로 수정
	int cnt_material;
	std::vector<GLuint *> textures;
	Shader shader; // pointer로 수정
	GLuint VAO, VBO;
	
	// control
	bool load_material;
	bool rotate_axis;

	// transform
	// ...

public:
	Object(cyTriMesh mesh, Shader shader, glm::vec3 translation, glm::vec3 scale, glm::mat4 projection);
	Object(cyTriMesh mesh, Shader shader, glm::vec3 translation, glm::vec3 scale, glm::mat4 projection, bool load_texture, bool flip);
	~Object();
	void draw();
	void draw(glm::mat4 view);
};

Object::Object(cyTriMesh mesh, Shader shader, glm::vec3 translation, glm::vec3 scale, glm::mat4 projection)
: mesh(mesh), cnt_material(0), shader(shader)
{
	;
}

Object::~Object()
{
}

void Object::draw()
{
}

void Object::draw(glm::mat4 view)
{
}

#endif
