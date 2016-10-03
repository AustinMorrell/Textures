#pragma once
#include <iostream>
#include "gl_core_4_4.h"
#include "Gizmos.h"
#include <GLFW\glfw3.h>
#include <glm\glm.hpp>
#include <glm\ext.hpp>
#include <fstream>
#include <string>
#include <vector>
#include <math.h>
#include <FBXFile.h>

class App
{
public:
	virtual bool start() = 0;
	virtual bool update() = 0;
	virtual void draw() = 0;
	virtual void destroy() = 0;
};

struct Vertex 
{
	float x, y, z, w;
	float nx, ny, nz, nw;
	float tx, ty, tz, tw;
	float s, t;
};

class Texture : public App {
public:
	Texture();
	bool start() override;
	bool update() override;
	void draw() override;
	void destroy() override;
	std::string ReadFromFile(std::string text);
	void createTriQuad();
	void makeModelBuffers(FBXFile* a);
	void makeData();
private:
	glm::mat4 m_projectionViewMatrix;
	glm::mat4 view;
	glm::mat4 projection;
	GLFWwindow* window;
	FBXFile* fbx;
	unsigned int m_programID;
	unsigned int m_texture;
	unsigned int p_VAO;
	unsigned int p_VBO;
	unsigned int p_IBO;
	unsigned int m_normal;
	int p_indicesCounter;
};