#define STB_IMAGE_IMPLEMENTATION

#include <stdio.h>
#include <string.h>
#include <cmath>
#include <vector>

#include <GL\glew.h>
#include <GLFW\glfw3.h>

#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <glm\gtc\type_ptr.hpp>

#include <assimp/Importer.hpp>

#include "CommonValues.h"

#include "Window.h"
#include "Mesh.h"
#include "Shader.h"
#include "Camera.h"
#include "Texture.h"
#include "DirectionalLight.h"
#include "PointLight.h"
#include "Material.h"
#include "Model.h"

const float toRadians = 3.14159265f / 180.0f;

Window mainWindow;
std::vector<Mesh*> meshList;
std::vector<Shader> shaderList;
Camera camera;

Texture plainTexture;

Material shinyMaterial;
Material dullMaterial;

Model Planet;


DirectionalLight mainLight;
PointLight pointLights[MAX_POINT_LIGHTS];

GLfloat deltaTime = 0.0f;
GLfloat lastTime = 0.0f;

// Vertex Shader
static const char* vShader = "Shaders/shader.vert";

// Fragment Shader
static const char* fShader = "Shaders/shader.frag";

void calcAverageNormals(unsigned int * indices, unsigned int indiceCount, GLfloat * vertices, unsigned int verticeCount, 
						unsigned int vLength, unsigned int normalOffset)
{
	for (size_t i = 0; i < indiceCount; i += 3)
	{
		unsigned int in0 = indices[i] * vLength;
		unsigned int in1 = indices[i + 1] * vLength;
		unsigned int in2 = indices[i + 2] * vLength;
		glm::vec3 v1(vertices[in1] - vertices[in0], vertices[in1 + 1] - vertices[in0 + 1], vertices[in1 + 2] - vertices[in0 + 2]);
		glm::vec3 v2(vertices[in2] - vertices[in0], vertices[in2 + 1] - vertices[in0 + 1], vertices[in2 + 2] - vertices[in0 + 2]);
		glm::vec3 normal = glm::cross(v1, v2);
		normal = glm::normalize(normal);
		
		in0 += normalOffset; in1 += normalOffset; in2 += normalOffset;
		vertices[in0] += normal.x; vertices[in0 + 1] += normal.y; vertices[in0 + 2] += normal.z;
		vertices[in1] += normal.x; vertices[in1 + 1] += normal.y; vertices[in1 + 2] += normal.z;
		vertices[in2] += normal.x; vertices[in2 + 1] += normal.y; vertices[in2 + 2] += normal.z;
	}

	for (size_t i = 0; i < verticeCount / vLength; i++)
	{
		unsigned int nOffset = i * vLength + normalOffset;
		glm::vec3 vec(vertices[nOffset], vertices[nOffset + 1], vertices[nOffset + 2]);
		vec = glm::normalize(vec);
		vertices[nOffset] = vec.x; vertices[nOffset + 1] = vec.y; vertices[nOffset + 2] = vec.z;
	}
}

void CreateCubes() 
{

	unsigned int indices[] = {
		0, 2, 3, 0, 3, 1,
		2, 6, 7, 2, 7, 3,
		6, 4, 5, 6, 5, 7,
		4, 0, 1, 4, 1, 5,
		0, 4, 6, 0, 6, 2,
		1, 5, 7, 1, 7, 3,
	};

	GLfloat vertices[] = {
		-1.0f,1.0f,1.0f,   0.0f,1.0f, 0.0f,0.0f,0.0f,
		-1.0f,-1.0f,1.0f,  0.0f,0.0f, 0.0f,0.0f,0.0f,
		 1.0f,1.0f,1.0f,   1.0f,1.0f, 0.0f,0.0f,0.0f,
		 1.0f,-1.0f,1.0f,  1.0f,0.0f, 0.0f,0.0f,0.0f,
		-1.0f,1.0f,-1.0f,  0.0f,1.0f, 0.0f,0.0f,0.0f,
		-1.0f,-1.0f,-1.0f, 0.0f,0.0f, 0.0f,0.0f,0.0f,
		 1.0f,1.0f,-1.0f,  1.0f,1.0f, 0.0f,0.0f,0.0f,
		 1.0f,-1.0f,-1.0f, 1.0f,0.0f, 0.0f,0.0f,0.0f,
	};

	
	calcAverageNormals(indices, 36, vertices, 64, 8, 5);

	Mesh *obj1 = new Mesh();
	obj1->CreateMesh(vertices, indices, 64, 36);
	meshList.push_back(obj1);

	Mesh *obj2 = new Mesh();
	obj2->CreateMesh(vertices, indices, 64, 36);
	meshList.push_back(obj2);

	Mesh *obj3 = new Mesh();
	obj3->CreateMesh(vertices,indices, 64, 36);
	meshList.push_back(obj3);

	Mesh* obj4 = new Mesh();
	obj4->CreateMesh(vertices, indices, 64, 36);
	meshList.push_back(obj4);

	Mesh* obj5 = new Mesh();
	obj5->CreateMesh(vertices, indices, 64, 36);
	meshList.push_back(obj5);

	Mesh* obj6 = new Mesh();
	obj6->CreateMesh(vertices, indices, 64, 36);
	meshList.push_back(obj6);
}



void CreateShaders()
{
	Shader *shader1 = new Shader();
	shader1->CreateFromFiles(vShader, fShader);
	shaderList.push_back(*shader1);
}

int main() 
{
	mainWindow = Window(1366, 768); 
	mainWindow.Initialise();

	CreateCubes();
	CreateShaders();

	camera = Camera(glm::vec3(0.0f, 5.0f, 40.0f), glm::vec3(0.0f, 1.0f, 0.0f), -90.0f, -10.0f, 5.0f, 0.5f);

	
	plainTexture = Texture("Textures/container.png");
	plainTexture.LoadTextureA();


	shinyMaterial = Material(4.0f, 256);
	dullMaterial = Material(0.3f, 3);
	
	Planet = Model();
	Planet.LoadModel("Models/planet.obj");

	mainLight = DirectionalLight(1.0f, 1.0f, 1.0f, 
								0.07f, 0.2f,
								0.0f, 0.0f, -1.0f);


	unsigned int pointLightCount = 0;
	pointLights[0] = PointLight(0.7f, 0.7f, 1.0f,
								1.3f, 1.0f,
								0.0f, 0.0f, 0.0f,
								0.6f, 0.3f, 0.05f);
	pointLightCount++;
	

	GLuint uniformProjection = 0, uniformModel = 0, uniformView = 0, uniformEyePosition = 0,
		uniformSpecularIntensity = 0, uniformShininess = 0;
	glm::mat4 projection = glm::perspective(glm::radians(45.0f), (GLfloat)mainWindow.getBufferWidth() / mainWindow.getBufferHeight(), 0.1f, 100.0f);

	int rotation_increment = 0.0f;


	// Loop until window closed
	while (!mainWindow.getShouldClose())
	{
		GLfloat now = glfwGetTime();
		deltaTime = now - lastTime; 
		lastTime = now;

		// Get + Handle User Input
		glfwPollEvents();

		camera.keyControl(mainWindow.getsKeys(), deltaTime);
			


		if (!mainWindow.getsKeys()[GLFW_KEY_SPACE]) {
			camera.mouseControl(mainWindow.getXChange(), mainWindow.getYChange());

			// Clear the window
			glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			shaderList[0].UseShader();
			uniformModel = shaderList[0].GetModelLocation();
			uniformProjection = shaderList[0].GetProjectionLocation();
			uniformView = shaderList[0].GetViewLocation();
			uniformEyePosition = shaderList[0].GetEyePositionLocation();
			uniformSpecularIntensity = shaderList[0].GetSpecularIntensityLocation();
			uniformShininess = shaderList[0].GetShininessLocation();

			shaderList[0].SetDirectionalLight(&mainLight);
			shaderList[0].SetPointLights(pointLights, pointLightCount);



			glUniformMatrix4fv(uniformProjection, 1, GL_FALSE, glm::value_ptr(projection));
			glUniformMatrix4fv(uniformView, 1, GL_FALSE, glm::value_ptr(camera.calculateViewMatrix()));
			glUniform3f(uniformEyePosition, camera.getCameraPosition().x, camera.getCameraPosition().y, camera.getCameraPosition().z);


			glm::mat4 model(1.0f);





			model = glm::rotate(model, (GLfloat)rotation_increment * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
			model = glm::translate(model, glm::vec3(15.0f, 0.0f, 0.0f));
			glm::mat4 planet_matrix(model);
			model = glm::rotate(model, (GLfloat)rotation_increment * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
			glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
			dullMaterial.UseMaterial(uniformSpecularIntensity, uniformShininess);
			Planet.RenderModel();

			model = glm::mat4(1.0f);
			model = model * planet_matrix;
			model = glm::rotate(model, (GLfloat)rotation_increment * toRadians, glm::vec3(0.0f, 5.0f, 0.0f));
			model = glm::translate(model, glm::vec3(5.0f, 0.0f, 0.0f));
			model = glm::rotate(model, (GLfloat)rotation_increment * toRadians, glm::vec3(0.0f, 0.0f, 1.0f));
			model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f));
			glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
			plainTexture.UseTexture();
			shinyMaterial.UseMaterial(uniformSpecularIntensity, uniformShininess);
			meshList[0]->RenderMesh();

			model = glm::mat4(1.0f);
			model = model * planet_matrix;

			model = glm::rotate(model, (GLfloat)rotation_increment * toRadians, glm::vec3(10.0f, 0.0f, 0.0f));
			model = glm::translate(model, glm::vec3(0.0f, 10.0f, 0.0f));
			model = glm::rotate(model, (GLfloat)rotation_increment * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
			model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f));
			glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
			plainTexture.UseTexture();
			shinyMaterial.UseMaterial(uniformSpecularIntensity, uniformShininess);
			meshList[1]->RenderMesh();

			model = glm::mat4(1.0f);
			model = model * planet_matrix;

			model = glm::rotate(model, (GLfloat)(60 * toRadians), glm::vec3(0.0f, 0.0f, 1.0f));
			model = glm::rotate(model, (GLfloat)rotation_increment * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
			model = glm::translate(model, glm::vec3(15.0f, 0.0f, 1.0f));
			model = glm::rotate(model, (GLfloat)rotation_increment * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
			model = glm::rotate(model, (GLfloat)(60 * toRadians), glm::vec3(0.0f, 1.0f, 0.0f));
			model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f));
			glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
			plainTexture.UseTexture();
			shinyMaterial.UseMaterial(uniformSpecularIntensity, uniformShininess);
			meshList[2]->RenderMesh();

			model = glm::mat4(1.0f);
			model = model * planet_matrix;

			model = glm::rotate(model, (GLfloat)(60 * toRadians), glm::vec3(0.0f, 0.0f, 1.0f));
			model = glm::rotate(model, (GLfloat)rotation_increment * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
			model = glm::translate(model, glm::vec3(-15.0f, 0.0f, 0.0f));
			model = glm::rotate(model, (GLfloat)rotation_increment * toRadians, glm::vec3(0.0f, 0.0f, 1.0f));
			model = glm::rotate(model, (GLfloat)(70 * toRadians), glm::vec3(1.0f, 0.0f, 0.0f));
			model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f));
			glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
			plainTexture.UseTexture();
			shinyMaterial.UseMaterial(uniformSpecularIntensity, uniformShininess);
			meshList[3]->RenderMesh();

			model = glm::mat4(1.0f);
			model = model * planet_matrix;

			model = glm::rotate(model, (GLfloat)rotation_increment * toRadians, glm::vec3(10.0f, 0.0f, 0.0f));
			model = glm::translate(model, glm::vec3(0.0f, -10.0f, 0.0f));
			model = glm::rotate(model, (GLfloat)rotation_increment * toRadians, glm::vec3(0.0f, 1.0f, 1.0f));
			model = glm::rotate(model, (GLfloat)(45 * toRadians), glm::vec3(0.0f, 0.0f, 1.0f));
			model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f));
			glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
			plainTexture.UseTexture();
			shinyMaterial.UseMaterial(uniformSpecularIntensity, uniformShininess);
			meshList[4]->RenderMesh();

			model = glm::mat4(1.0f);
			model = model * planet_matrix;
			model = glm::rotate(model, (GLfloat)rotation_increment * toRadians, glm::vec3(0.0f, 5.0f, 0.0f));
			model = glm::translate(model, glm::vec3(-5.0f, 0.0f, 0.0f));
			model = glm::rotate(model, (GLfloat)rotation_increment * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
			model = glm::rotate(model, (GLfloat)(120 * toRadians), glm::vec3(0.0f, 1.0f, 1.0f));
			model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f));
			glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
			plainTexture.UseTexture();
			shinyMaterial.UseMaterial(uniformSpecularIntensity, uniformShininess);
			meshList[5]->RenderMesh();

			pointLights[0].UpdateLight(planet_matrix);
			shaderList[0].SetPointLights(pointLights, pointLightCount);

			
			glUseProgram(0);

			mainWindow.swapBuffers();

			rotation_increment += 1.0f;
		}
		
	}

	return 0;
}