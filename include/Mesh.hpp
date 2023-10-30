#ifndef MESH_HPP
#define MESH_HPP

class Mesh
{
private:
	/* data */
public:
	struct triangle // triangle
	{
		unsigned int indices[3];
	};

	Mesh(/* args */);
	~Mesh();
};

Mesh::Mesh(/* args */)
{
}

Mesh::~Mesh()
{
}

#endif
