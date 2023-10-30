#ifndef OBJECT_HPP
#define OBJECT_HPP

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <vector>

#include "stb_image.h"
#include "Mesh.hpp"
#include "Shader.hpp"
#include "cyTriMesh.h"

class Object
{
private:
	cyTriMesh mesh; // pointer로 수정
	int cnt_material;
	std::vector<GLuint*> textures;
	Shader shader; // pointer로 수정
	GLuint VAO, VBO;
	
	// control
	bool load_material;
	bool rotate_axis;

	// transform
    float yaw = 0;
    float pitch = 0;
    float cameraDistance = 0;
    glm::vec4 lightPos = glm::vec4(0, 0, 0, 1);
    glm::mat4 model = glm::mat4(1.0f);
    glm::mat4 projection = glm::mat4(1.0f);
    glm::vec3 translation = glm::vec3(0, 0, 0);
    glm::vec3 scale = glm::vec3(1, 1, 1);
    glm::mat4 view = glm::mat4(1.0f);

    glm::mat4 cubeMapOrientation = glm::mat4(1.0f);

	Object();

    void ManageTextures(GLuint* textures, cyTriMesh ctm, size_t materialNum)
    {
        if (ctm.M(materialNum).map_Kd) {
            printf("before: %d\n", textures[0]);
            glGenTextures(1, &textures[0]);
            printf("after: %d\n", textures[0]);
            glBindTexture(GL_TEXTURE_2D, textures[0]);
            SetTextureData(ctm.M(materialNum).map_Kd);
        }
        if (ctm.M(materialNum).map_Ks) {
            printf("before: %d\n", textures[1]);
            glGenTextures(1, &textures[1]);
            printf("after: %d\n", textures[1]);
            glBindTexture(GL_TEXTURE_2D, textures[1]);
            SetTextureData(ctm.M(materialNum).map_Ks);
        }
    }

    void SetTextureData(const char* name) {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        int width, height, nrChannels;
        unsigned char* data = stbi_load(("Models/" + std::string(name)).c_str(), &width, &height, &nrChannels, 0);
        if (data)
        {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            glGenerateMipmap(GL_TEXTURE_2D);
        }
        else
        {
            std::cout << "Failed to load texture " << name << std::endl;
        }
        stbi_image_free(data);
    }

public:
	Object(cyTriMesh mesh, Shader shader, \
        glm::vec3 translation, glm::vec3 scale, glm::mat4 projection, \
        bool load_material = true, bool rotate_axis = true) \
        : mesh(mesh), cnt_material(0), shader(shader), \
        projection(projection), translation(translation), scale(scale), \
        load_material(load_material), rotate_axis(rotate_axis)
    {

        model = glm::mat4(1.0f);
        model = glm::translate(model, translation);
        if (rotate_axis) {
            model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1, 0, 0));
        }

        stbi_set_flip_vertically_on_load(true);

#pragma region Vertex
        mesh.ComputeBoundingBox();
        cyVec3f boundMiddle = (mesh.GetBoundMax() + mesh.GetBoundMin()) / 2.0f;
        float ModelSize = (mesh.GetBoundMax() - mesh.GetBoundMin()).Length() / 20.0f;

        unsigned int attributeSize = 3;
        if (mesh.HasNormals()) { attributeSize += 3; }
        if (mesh.HasTextureVertices()) { attributeSize += 2; }
        std::vector<float> vertices;
        for (size_t i = 0; i < mesh.NF(); i++)
        {
            for (size_t j = 0; j < 3; j++)
            {
                //vertices
                vertices.push_back((mesh.V(mesh.F(i).v[j]).x - boundMiddle.x) / ModelSize * scale.x);
                vertices.push_back((mesh.V(mesh.F(i).v[j]).y - boundMiddle.y) / ModelSize * scale.y);
                vertices.push_back((mesh.V(mesh.F(i).v[j]).z - boundMiddle.z) / ModelSize * scale.z);
                //normals
                if (mesh.HasNormals()) {
                    vertices.push_back(mesh.VN(mesh.FN(i).v[j]).x);
                    vertices.push_back(mesh.VN(mesh.FN(i).v[j]).y);
                    vertices.push_back(mesh.VN(mesh.FN(i).v[j]).z);
                    //texture coordinates
                    if (mesh.HasTextureVertices()) {
                        vertices.push_back(mesh.VT(mesh.FT(i).v[j]).x);
                        vertices.push_back(mesh.VT(mesh.FT(i).v[j]).y);
                    }
                }
            }
        }

        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, attributeSize * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        if (mesh.HasNormals()) {
            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, attributeSize * sizeof(float), (void*)(3 * sizeof(float)));
            glEnableVertexAttribArray(1);
            if (mesh.HasTextureVertices()) {
                glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, attributeSize * sizeof(float), (void*)(6 * sizeof(float)));
                glEnableVertexAttribArray(2);
            }
        }

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
#pragma endregion

#pragma region Materials & Textures
        if (load_material) {
            cnt_material = mesh.NM();
            for (size_t i = 0; i < cnt_material; i++)
            {
                printf("???\n");
                GLuint* textureTmp = new GLuint[2];
                ManageTextures(textureTmp, mesh, i);
                textures.push_back(textureTmp);
            }
            shader.use();
            shader.setInt("diffuseTexture", 0);
            shader.setInt("specularTexture", 1);
        }
#pragma endregion  
    }
	
	~Object();
	void draw();
	void draw(glm::mat4 view);
};


Object::~Object()
{
}

void Object::draw()
{
	shader.use();
	glBindVertexArray(VAO);
	glDrawArrays(GL_TRIANGLES, 0, mesh.NF() * 3);
}

void Object::draw(glm::mat4 view)
{
	shader.use();
	shader.setMat4("mvp", projection * view * model);
	shader.setMat4("mv", view * model);
    shader.setMat4("mvN", glm::transpose(glm::inverse(view * model)));   //mv for Normals

	// TODO: `
	if (cnt_material == 0 || !load_material) {
		glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLES, 0, mesh.NF() * 3);
	}
	else {
		for (size_t i = 0; i < cnt_material; i++)
		{
			GLuint* t = textures[i];

			if (mesh.M(i).Ns) {
				shader.setFloat("specularExponent", mesh.M(i).Ns);
			}
			/*if (mesh.M(i).Ka) {
				shader.setVec3("ka", glm::vec3(mesh.M(i).Ka[0], mesh.M(i).Ka[1], mesh.M(i).Ka[2]));
			}*/
			if (mesh.M(i).Kd) {
				shader.setVec3("kd", glm::vec3(mesh.M(i).Kd[0], mesh.M(i).Kd[1], mesh.M(i).Kd[2]));
			}
			if (mesh.M(i).Ks) {
				shader.setVec3("ks", glm::vec3(mesh.M(i).Ks[0], mesh.M(i).Ks[1], mesh.M(i).Ks[2]));
			}

			shader.setBool("use_tex_d", true);
			shader.setBool("use_tex_s", true);                       
			if (!mesh.M(i).map_Kd) {
				shader.setBool("use_tex_d", false);
			}
			if (!mesh.M(i).map_Ks) {
				shader.setBool("use_tex_s", false);
			}

static int p = false;
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, t[0]);
            if (p == false)
            {
                printf("%d %d\n", t[0], t[1]);
                printf("Error: %d\n", glGetError());
            }
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, t[1]);
            if (p == false)
            {
                printf("%d %d\n", t[0], t[1]);
                printf("Error: %d\n", glGetError());
                p = true;
            }

			glBindVertexArray(VAO);
			glDrawArrays(GL_TRIANGLES, mesh.GetMaterialFirstFace(i) * 3, mesh.GetMaterialFaceCount(i) * 3);
		}
	}
}

#endif
