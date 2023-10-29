#include <glad/glad.h> 
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <vector>
#include <string>
#include <memory>

#include "Quad.hpp"
#include "Object.hpp"
#include "Shader.hpp"
#include "Mesh.hpp"
#include "Camera.hpp"
#include "cyTriMesh.h"

// TODO: FBO 클래스 생성 (텍스처 포인터의 벡터 가짐)

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);
void mouseMovementCallback(GLFWwindow* window, double xpos, double ypos);

const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;

std::vector<Object *> objects;
Camera camera(glm::vec3(0, 0, 30.0f));

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, false);

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Yonazang's SSR", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouseMovementCallback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glEnable(GL_STENCIL_TEST);
    glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

    Quad *quad = new Quad();

    // 디폴트 FBO 백업
    GLint default_FBO;
    glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &default_FBO);

    // g-buffer FBO
    GLuint gBuffer_FBO;
    glGenFramebuffers(1, &gBuffer_FBO);
    glBindFramebuffer(GL_FRAMEBUFFER, gBuffer_FBO);
    
    GLuint gPosition, gNormal, gAlbedo, gSpecular;

    glGenTextures(1, &gNormal);
    glBindTexture(GL_TEXTURE_2D, gNormal);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gNormal, 0);

    glGenTextures(1, &gAlbedo);
    glBindTexture(GL_TEXTURE_2D, gAlbedo);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gAlbedo, 0);

    glGenTextures(1, &gSpecular);
    glBindTexture(GL_TEXTURE_2D, gSpecular);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gSpecular, 0);

	// 하나의 텍스처에 depth + stencil 둘다 넣음
    // stencil 필요한지 확인 필요
    GLuint depthMap;
    glGenTextures(1, &depthMap);
    glBindTexture(GL_TEXTURE_2D, depthMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH32F_STENCIL8, SCR_WIDTH, SCR_HEIGHT, 0, GL_DEPTH_STENCIL, GL_FLOAT_32_UNSIGNED_INT_24_8_REV, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, depthMap, 0);

    GLenum drawBuffers1[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
    glDrawBuffers(3, drawBuffers1);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cout << std::hex << glCheckFramebufferStatus(GL_FRAMEBUFFER) << std::endl;
        return -1;
    }

	GLuint lighting_FBO; // Lighting Pass FrameBuffer
	glGenFramebuffers(1, &lighting_FBO);
	glBindFramebuffer(GL_FRAMEBUFFER, lighting_FBO);
	GLuint colorBuffer;
    glGenTextures(1, &colorBuffer);
    glBindTexture(GL_TEXTURE_2D, colorBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorBuffer, 0);
	// glFramebufferTexture 2D와 아닌 것 차이점

    GLenum drawBuffers2[1] = { GL_COLOR_ATTACHMENT0 };
    glDrawBuffers(1, drawBuffers2);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cout << std::hex << glCheckFramebufferStatus(GL_FRAMEBUFFER) << std::endl;
        return -1;
    }

	GLuint SSR_FBO; // Screen Space Reflection FrameBuffer
	glGenFramebuffers(1, &SSR_FBO);
	glBindFramebuffer(GL_FRAMEBUFFER, SSR_FBO);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, depthMap, 0);

	GLuint reflectionColorBuffer;
	glGenTextures(1, &reflectionColorBuffer);
	glBindTexture(GL_TEXTURE_2D, reflectionColorBuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, reflectionColorBuffer, 0);
    GLenum drawBuffers3[1] = { GL_COLOR_ATTACHMENT0 };
    glDrawBuffers(1, drawBuffers3);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cout << "problems binding ssrFB" << std::endl;
        return -1;
    }

	Shader forward_pass_shader("Shaders/ForwardPass.vs", "Shaders/ForwardPass.fs");
    int cnt_models = 3;
    cyTriMesh *object_mesh = new cyTriMesh();
    const char *object_names[] = {
        "bunny",
        "SIMPLE ROUND TABLE",
        "teapot",
        "cube"
    };
    glm::vec3 obejct_transformations[] = {
        // translate                    // scale
        glm::vec3(0.0f, 5.0f, -20.0f), glm::vec3(3.0f, 3.0f, 3.0f), 
        glm::vec3(-17.0f, -9.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f),
        glm::vec3(-17.0f, -2.0f, 0.0f), glm::vec3(0.5f, 0.5f, 0.5f),
        glm::vec3(0.0f, -9.0f, 0.0f), glm::vec3(0.6f, 1.0f, 0.6f),
    };
    glm::mat4 projection = glm::mat4(1.0f);
    projection = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 300.0f);
	bool object_flip[] = {
		false,
		false,
		true,
		false,
	};
    for (int i = 0; i < cnt_models; i++)
    {
        if (object_mesh->LoadFromFileObj("filename") == false) {
            return (-1);
        }  
        objects.push_back(\
            new Object(*object_mesh, forward_pass_shader, \
            obejct_transformations[i * 2], obejct_transformations[i * 2 + 1], projection));
    }
    delete object_mesh;

    // CHECK: 음? ground가 뭘까... SSR에서 사용하는 객체?
    cyTriMesh ground_mesh;
    if (ground_mesh.LoadFromFileObj("Models/ground.obj") == false) {
        return -1;
    }   
    Object *ground = new Object(ground_mesh, forward_pass_shader, glm::vec3(0.0f, -20.0f, 0.0f), glm::vec3(10.0f, 1.0f, 10.0f), projection);

    Shader lighting_pass_shader("Shaders/DeferredPass.vs", "Shaders/DeferredPass.fs");
    lighting_pass_shader.use();
    lighting_pass_shader.setInt("gNormal", 0);
    lighting_pass_shader.setInt("gAlbedo", 1);
    lighting_pass_shader.setInt("gSpecular", 2);
    lighting_pass_shader.setInt("depthMap", 3);
    // CHECK: constat, linear, quadratic의 의미
    glm::vec3 lights[] = {
        // position				// color						// constant, linear, quadratic
        glm::vec3(20, 10, 10),    glm::vec3(1,1,1),               glm::vec3(1,0.007f,0.0002f),          //position, color, (constant, linear, quadratic)
        glm::vec3(-25, -5, -35), glm::vec3(0.224f,0.42f,0.659f), glm::vec3(1,0.007f,0.0002f),
        glm::vec3(25, -5, -35), glm::vec3(0.306f,0.714f,0.71f), glm::vec3(1,0.027,0.0028),
	};
	// setting lights
    for (int i = 0; i < sizeof(lights) / sizeof(lights[0]); i++)
	{
		lighting_pass_shader.setVec3("lights[" + std::to_string(i) + "].intensity", lights[(3 * i) + 1]);	
		lighting_pass_shader.setFloat("lights[" + std::to_string(i) + "].constant", lights[(3 * i) + 2].x);	
		lighting_pass_shader.setFloat("lights[" + std::to_string(i) + "].linear", lights[(3 * i) + 2].x);	
		lighting_pass_shader.setFloat("lights[" + std::to_string(i) + "].quadratic", lights[(3 * i) + 2].x);	
	}

	Shader SSR_shader("Shaders/SSR.vs", "Shaders/SSR.fs");
	SSR_shader.use();
	// set texture uniform value
	SSR_shader.setInt("gNormal", 0);
	SSR_shader.setInt("colorBuffer", 1);
	SSR_shader.setInt("depthMap", 2);

	// CHECK: 왜 SSR 버텍스쉐이더지?
	Shader output_shader("Shaders/SSR.vs", "Shaders/output.fs");
	output_shader.use();
	output_shader.setInt("colorTexture", 0);
	output_shader.setInt("refTexture", 1);
	output_shader.setInt("specularTexture", 2);
    
    /*
    1. forward pass (geometry) -> gBuffer_FBO
    2. deferred pass (lighting) -> lighting_FBO
    3. SSR pass -> SSR_FBO
    4. output pass -> default_FBO
    */
    while (!glfwWindowShouldClose(window))
    {
        processInput(window);
        // TODO: Frame 관련 처리(deltaTime 구해서 지연 반영?)

        // 렌더링 기본 세팅
        glClearColor(0, 0, 0, 1.0f);
        glEnable(GL_DEPTH_TEST);

        // CHECK: stencil 어떻게 사용되는지 확인 필요
        // forward pass (geomerty pass)
        glBindFramebuffer(GL_FRAMEBUFFER, gBuffer_FBO);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        // 오브젝트 및 땅 그려주기
	    glm::mat4 view = camera.getViewMatrix();
        for (std::vector<Object *>::iterator it = objects.begin(); it != objects.end(); it++)
        {
            (*it)->draw(view);
        }
        ground->draw(view);

        // deferred(lighting) pass
        glBindFramebuffer(GL_FRAMEBUFFER, lighting_FBO);
        glClear(GL_COLOR_BUFFER_BIT);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, gNormal);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, gAlbedo);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, gSpecular);
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, depthMap);

        lighting_pass_shader.use();
        lighting_pass_shader.setVec3("lightPosition", view * glm::vec4(lights[0], 1));
        lighting_pass_shader.setFloat("SCR_WIDTH", SCR_WIDTH);
        lighting_pass_shader.setFloat("SCR_HEIGHT", SCR_HEIGHT);
        lighting_pass_shader.setMat4("invProj", glm::inverse(projection));
        for (int i = 0; i < sizeof(lights) / sizeof(lights[0]); i++)
        {
            lighting_pass_shader.setVec3("lights[" + std::to_string(i) + "].position", view * glm::vec4(lights[i * 3], 1));
        }
        glBindVertexArray(quad->GetVAO());
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        // SSR pass
        // 노말, 컬러, 깊이 필요
        glBindFramebuffer(GL_FRAMEBUFFER, SSR_FBO);
        glClear(GL_COLOR_BUFFER_BIT);
        glDepthMask(GL_FALSE); // depth test를 수행하되 depth buffer의 내용은 수정하지 않는다(read-only)

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, gNormal);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, colorBuffer);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, depthMap);

        SSR_shader.use();
        SSR_shader.setFloat("SCR_WIDTH", SCR_WIDTH);
        SSR_shader.setFloat("SCR_HEIGHT", SCR_HEIGHT);
        SSR_shader.setMat4("invProjection", glm::inverse(projection));
        SSR_shader.setMat4("projection", projection);

        glBindVertexArray(quad->GetVAO());
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glDepthMask(GL_TRUE); // 설정 원상복귀

        // output (lighting pass와 SSR pass의 결과물을 합친다)
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, default_FBO); // 그냥 두번째 인자 0으로 해도 될듯?
        glClear(GL_COLOR_BUFFER_BIT);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, colorBuffer);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, reflectionColorBuffer);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, gSpecular);

        output_shader.use();
        glBindVertexArray(quad->GetVAO());
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // clear
    for (std::vector<Object *>::iterator it = objects.begin(); it != objects.end(); it++)
    {
        delete (*it);
    }
    delete ground;
    delete quad;
    glDeleteFramebuffers(1, &gBuffer_FBO);
    glDeleteFramebuffers(1, &lighting_FBO);
    glDeleteFramebuffers(1, &SSR_FBO);
    glfwTerminate();
    return (0);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
}

void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        camera.move(FORWARD, camera.speed);
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        camera.move(BACKWARD, camera.speed);
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        camera.move(LEFT, camera.speed);
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        camera.move(RIGHT, camera.speed);
    }
}

void mouseMovementCallback(GLFWwindow* window, double xpos, double ypos)
{
    camera.rotate(xpos, ypos);
}
