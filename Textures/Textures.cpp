#include <stb_image.h>
#include "app.h"

Texture::Texture()
{
	glfwInit();

	window = glfwCreateWindow(1280, 720, "Rendering Geometry", nullptr, nullptr);

	if (window == nullptr)
	{
		glfwTerminate();
	}

	glfwMakeContextCurrent(window);

	if (ogl_LoadFunctions() == ogl_LOAD_FAILED)
	{
		glfwDestroyWindow(window);
		glfwTerminate();
	}

	view = glm::lookAt(glm::vec3(15, 15, 15), glm::vec3(0), glm::vec3(0, 1, 0));
	projection = glm::perspective(glm::pi<float>() * 0.25f, 16 / 9.f, 0.1f, 1000.f);

	glEnable(GL_BLEND);
	glClearColor(0.25f, 0.25f, 0.25f, 1);
	glEnable(GL_DEPTH_TEST);
}

bool Texture::start()
{
	/*createTriQuad();*/
	makeData();

	fbx = new FBXFile();
	fbx->load("data/models/stanford/Dragon.fbx");
	makeModelBuffers(fbx);

	int imageWidth = 0, imageHeight = 0, imageFormat = 0;
	unsigned char* data;

	data = stbi_load("data/textures/rock_diffuse.tga", &imageWidth, &imageHeight, &imageFormat, STBI_default);
	glGenTextures(1, &m_texture);
	glBindTexture(GL_TEXTURE_2D, m_texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, imageWidth, imageHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	stbi_image_free(data);

	data = stbi_load("data/textures/rock_normal.tga", &imageWidth, &imageHeight, &imageFormat, STBI_default);
	glGenTextures(1, &m_normal);
	glBindTexture(GL_TEXTURE_2D, m_normal);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, imageWidth, imageHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	stbi_image_free(data);

	/*glGenTextures(1, &m_texture);
	glBindTexture(GL_TEXTURE_2D, m_texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, imageWidth, imageHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	stbi_image_free(data);*/

	const char* vsSource;
	std::string vs = ReadFromFile("vsInfo.txt");
	vsSource = vs.c_str();

	const char* fsSource;
	std::string fs = ReadFromFile("fsInfo.txt");
	fsSource = fs.c_str();

	/*int success = GL_FALSE;*/
	unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
	unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

	glShaderSource(vertexShader, 1, (const char**)&vsSource, 0);
	glCompileShader(vertexShader);
	glShaderSource(fragmentShader, 1, (const char**)&fsSource, 0);
	glCompileShader(fragmentShader);

	m_programID = glCreateProgram();
	glAttachShader(m_programID, vertexShader);
	glAttachShader(m_programID, fragmentShader);
	glLinkProgram(m_programID);

	glDeleteShader(fragmentShader);
	glDeleteShader(vertexShader);

	return true;
}

bool Texture::update()
{
	while (glfwWindowShouldClose(window) == false && glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS)
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		return true;
	}
	return false;
}

void Texture::draw()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glUseProgram(m_programID);

	m_projectionViewMatrix = projection * view;
	int loc = glGetUniformLocation(m_programID, "ProjectionView");
	glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(m_projectionViewMatrix));

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_texture);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, m_normal);

	loc = glGetUniformLocation(m_programID, "diffuse");
	glUniform1i(loc, 0);

	loc = glGetUniformLocation(m_programID, "normal");
	glUniform1i(loc, 1);

	glm::vec3 lightAD(sin(glfwGetTime()), 1, cos(glfwGetTime()));
	loc = glGetUniformLocation(m_programID, "LightDir");
	glUniform3f(loc, lightAD.x, lightAD.y, lightAD.z);

	glBindVertexArray(p_VAO);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);

	for (unsigned int i = 0; i < fbx->getMeshCount(); ++i) 
	{
		FBXMeshNode* mesh = fbx->getMeshByIndex(i);
		unsigned int* glData = (unsigned int*)mesh->m_userData;
		glBindVertexArray(glData[0]);
		glDrawElements(GL_TRIANGLES, (unsigned int)mesh->m_indices.size(), GL_UNSIGNED_INT, 0);
	}

	glBindVertexArray(p_VAO);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);

	glfwSwapBuffers(window);
	glfwPollEvents();
}

void Texture::destroy()
{
	glfwDestroyWindow(window);
	glfwTerminate();
}

std::string Texture::ReadFromFile(std::string text)
{
	std::string data;
	std::string container;
	std::ifstream file(text);

	if (file.is_open())
	{
		while (std::getline(file, data))
		{
			container += data + "\n";
		}
		file.close();
	}
	return container;
}

void Texture::createTriQuad()
{
	float vertexData[] = {
		-5, 0, 5, 1, 0, 1,
		5, 0, 5, 1, 1, 1,
		5, 0, -5, 1, 1, 0,
		-5, 0, -5, 1, 0, 0,
	};

	unsigned int indexData[] = {
		0, 1, 2,
		0, 2, 3,
	};

	glGenVertexArrays(1, &p_VAO);
	glBindVertexArray(p_VAO);

	glGenBuffers(1, &p_VBO);
	glBindBuffer(GL_ARRAY_BUFFER, p_VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, vertexData, GL_STATIC_DRAW);

	glGenBuffers(1, &p_IBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, p_IBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * 6, indexData, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(float) * 6, 0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 6, ((char*)0) + 16);
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void Texture::makeData()
{
	Vertex vertexData[] = {
		{ -5, 0, 5, 1, 0, 1, 0, 0, 1, 0, 0, 0, 0, 1 },
		{ 5, 0, 5, 1, 0, 1, 0, 0, 1, 0, 0, 0, 1, 1 },
		{ 5, 0, -5, 1, 0, 1, 0, 0, 1, 0, 0, 0, 1, 0 },
		{ -5, 0, -5, 1, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0 },
	};
	unsigned int indexData[] = {
		0, 1, 2,
		0, 2, 3,
	};
	glGenVertexArrays(1, &p_VAO);
	glBindVertexArray(p_VAO);

	glGenBuffers(1, &p_VBO);
	glBindBuffer(GL_ARRAY_BUFFER, p_VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * 4, vertexData, GL_STATIC_DRAW);

	glGenBuffers(1, &p_IBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, p_IBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * 6, indexData, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), ((char*)0) + 48);
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), ((char*)0) + 16);
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), ((char*)0) + 32);
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void Texture::makeModelBuffers(FBXFile* a)
{
	for (unsigned int i = 0; i < a->getMeshCount(); ++i)
	{
		FBXMeshNode* mesh = a->getMeshByIndex(i);
		unsigned int* glData = new unsigned int[3];

		glGenVertexArrays(1, &glData[0]);
		glBindVertexArray(glData[0]);
		glGenBuffers(1, &glData[1]);
		glGenBuffers(1, &glData[2]);
		glBindBuffer(GL_ARRAY_BUFFER, glData[1]);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, glData[2]);
		glBufferData(GL_ARRAY_BUFFER, mesh->m_vertices.size() * sizeof(FBXVertex), mesh->m_vertices.data(), GL_STATIC_DRAW);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh->m_indices.size() * sizeof(unsigned int), mesh->m_indices.data(), GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(FBXVertex), 0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 4, GL_FLOAT, GL_TRUE, sizeof(FBXVertex), ((char*)0) + FBXVertex::NormalOffset);
		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		mesh->m_userData = glData;
	}
}