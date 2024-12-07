#include <stdio.h>
#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "ModelImporter.h"
#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <cmath>
#define STB_IMAGE_IMPLEMENTATION 
#include "stb_image.h" 

#define TREE1 0
#define TREE2 1
#define SQUARE 2
#define LIB 3
#define MODELNUM 4

using namespace std;

// 窗口参数
int wWidth = 800, wHeight = 600;

// VAO参数
static const int numVAOs = MODELNUM;
GLuint vao[numVAOs] = { 0 };

// 着色器和纹理参数
GLuint shaderProgram = 0;
GLuint texture[MODELNUM] = { 0 };

//鼠标参数
double xCurPos = 0.0f;
double yCurPos = 0.0f;
double xNewPos = 0.0f;
double yNewPos = 0.0f;

// 输入参数
bool mouseStatus[3] = { false, false, false };
int keyStatus[512] = { 0 };

// 矩阵参数
glm::mat4 modelMatrix(1.0f);
glm::mat4 projectionMatrix(1.0f);
glm::mat4 viewMatrix(1.0f);

// 相机参数
glm::vec3 cameraPos = glm::vec3(1.0f, 1.0f, 0.5f);
glm::vec3 cameraFront(1.0f);
glm::vec3 cameraUp(1.0f);
glm::vec3 cameraRight(1.0f);
glm::vec3 worldUp = glm::vec3(0.0f, 1.0f, 0.0f);

// 光点位置
float lightPos = 50.0f;
float theta = 0.0f;

// 移动参数
glm::vec3 moveFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 moveRight = glm::vec3(1.0f, 0.0f, 0.0f);

// 视角参数
float pitch = 0.0f;
float yaw = -90.0f;

// 操控参数
float cameraSpeed = 0.8f;
float sensitivity = 0.3f;

// 模型参数
ModelImporter models[10];
char* modelFiles[] = { "./model/tree1.obj", "./model/tree2.obj", "./model/square.obj", "./model/lib.obj"};
char* textureFiles[] = { "./model/tree1.png", "./model/tree2.png", "./model/square.png","./model/lib.png" };

// 顶点着色器源码
const char* vertexShaderSource = "#version 460 core\n"
"layout(location = 0) in vec3 aPos;\n"
"layout(location = 1) in vec2 aUV;\n"
"layout(location = 2) in vec3 aNormal;\n"
"uniform mat4 model;\n"
"uniform mat4 view;\n"
"uniform mat4 projection;\n"
"out vec2 uv;\n"
"out vec3 normal;\n"
"out vec3 worldPos;\n"
"void main()\n"
"{\n"
"worldPos = vec3(model * vec4(aPos, 1.0)); \n"
"vec4 wpos = model * vec4(aPos, 1.0);\n"
"vec4 viewPos = view * wpos;\n"
"vec4 clipPos = projection * viewPos;\n"
"gl_Position = clipPos;\n"
"uv = aUV;\n"
"normal = aNormal;\n"
"}\n\0";

// 片段着色器源码
const char* fragmentShaderSource = "#version 460 core\n"
"out vec4 FragColor;\n"
"uniform sampler2D sampler;\n"
"uniform vec3 lightPos;\n"
"uniform vec3 viewPos;\n"
"in vec2 uv;\n"
"in vec3 normal;\n"
"in vec3 worldPos;\n"
"void main()\n"
"{\n"
"vec3 objectColor = vec3(texture(sampler, uv));\n"
"float ambientStrength = 0.1;\n"
"vec3 ambient = ambientStrength * objectColor;\n"
"vec3 norm = normalize(normal);\n"
"vec3 lightDir = normalize(lightPos - worldPos);\n"
"float diff = max(dot(norm, lightDir), 0.0);\n"
"vec3 diffuse = diff * objectColor;\n"
"float specularStrength = 0.5;\n"
"vec3 viewDir = normalize(viewPos - worldPos);\n"
"vec3 reflectDir = reflect(-lightDir, norm);\n"
"float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);\n"
"vec3 specular = specularStrength * spec * objectColor;\n"
"vec3 result = (ambient + diffuse + specular) * objectColor;\n"
"FragColor = vec4(result, 1.0); \n"
"}\n\0";

void frameBufferSizeCallBack(GLFWwindow* window, int width, int height) {
	wWidth = width;
	wHeight = height;
	if (wWidth == 0 || wHeight == 0) {
		cout << "窗口关闭" << endl;
		exit(EXIT_SUCCESS);
	}
	glViewport(0, 0, wWidth, wHeight);
}

void keyCallBack(GLFWwindow* window, int key, int scancode, int action, int modes) {
	if (action == GLFW_PRESS) {
		keyStatus[key] = 1;
	}
	else if (action == GLFW_RELEASE) {
		keyStatus[key] = 0;
	}
}

void mouseCallBack(GLFWwindow* window, int button, int action, int modes) {
	if (button == GLFW_MOUSE_BUTTON_LEFT || button == GLFW_MOUSE_BUTTON_RIGHT || button == GLFW_MOUSE_BUTTON_MIDDLE) {
		if (action == GLFW_PRESS) {
			mouseStatus[button] = true;
		}
		else if (action == GLFW_RELEASE) {
			mouseStatus[button] = false;
		}
	}
}

void mouseMoveCallBack(GLFWwindow* window, double xpos, double ypos) {
	xNewPos = xpos <= wWidth ? xpos : wWidth;
	xNewPos = xNewPos < 0 ? 0 : xNewPos;
	yNewPos = ypos <= wHeight ? ypos : wHeight;
	yNewPos = yNewPos < 0 ? 0 : yNewPos;
}

void prepareStatus() {
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
}

void prepareShader() {
	// 编译着色器
	int vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
	glCompileShader(vertexShader);
	int success;
	char infoLog[512];
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
	}

	int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
	glCompileShader(fragmentShader);
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
	}

	// 链接着色器
	shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);
	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
	}
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
}

void prepareTexture() {
	int width, height, channel;
	// 加载图片
	for (int i = 0; i < MODELNUM; i++) {
		stbi_set_flip_vertically_on_load(true);
		unsigned char* data = stbi_load(textureFiles[i], &width, &height, &channel, STBI_rgb_alpha);
		// 创建纹理对象
		glGenTextures(1, &(texture[i]));
		// 激活纹理单元
		glActiveTexture(GL_TEXTURE0 + i);
		// 绑定纹理对象
		glBindTexture(GL_TEXTURE_2D, texture[i]);

		// 设置纹理的过滤方式
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);

		//设置纹理的包裹方式
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); // u
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT); // v

		//传输数据
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		//释放图片
		stbi_image_free(data);
	}
}

void prepareVAO(ModelImporter model, int i)
{
	int numVBOs = 3;
	GLuint vbo[3] = { 0 };

	vector<float> pValues = model.getVertices();
	vector<float> tValues = model.getTextureCoordinates();
	vector<float> nValues = model.getNormals();

	// 创建VBO
	glGenBuffers(numVBOs, vbo);

	// 提交数据到VBO
	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	glBufferData(GL_ARRAY_BUFFER, pValues.size() * sizeof(float), &(pValues[0]), GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
	glBufferData(GL_ARRAY_BUFFER, tValues.size() * sizeof(float), &(tValues[0]), GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[2]);
	glBufferData(GL_ARRAY_BUFFER, nValues.size() * sizeof(float), &(nValues[0]), GL_STATIC_DRAW);

	// 绑定VAO
	glBindVertexArray(vao[i]);

	// 在VAO中配置VBO属性
	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

	// 解绑VAO
	glBindVertexArray(0);
}

void prepareModelsAndVAO() {
	glGenVertexArrays(numVAOs, vao);
	for (int i = 0; i < MODELNUM; i++) {
		char* path = modelFiles[i];
		ModelImporter model;
		model.parseOBJ(path);
		prepareVAO(model, i);
		models[i] = model;
	}
}

void processCameraMat() {
	glm::vec3 front, mFront;
	front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	front.y = sin(glm::radians(pitch));
	front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	cameraFront = glm::normalize(front);
	cameraRight = glm::normalize(glm::cross(cameraFront, worldUp)); 
	cameraUp = glm::normalize(glm::cross(cameraRight, cameraFront));
	mFront.x = cos(glm::radians(yaw));
	mFront.z = sin(glm::radians(yaw));
	mFront.y = 0.0f;
	moveFront = glm::normalize(mFront);
	moveRight = glm::normalize(glm::cross(moveFront, worldUp));
}

void processKeyInfo() {
	if (keyStatus[GLFW_KEY_W]) {
		cameraPos += cameraSpeed * cameraFront;
	}
	if (keyStatus[GLFW_KEY_S]) {
		cameraPos -= cameraSpeed * cameraFront;
	}
	if (keyStatus[GLFW_KEY_A]) {
		cameraPos -= cameraSpeed * moveRight;
	}
	if (keyStatus[GLFW_KEY_D]) {
		cameraPos += cameraSpeed * moveRight;
	}
	if (keyStatus[GLFW_KEY_H]) {
		theta += 0.2f;
	}
}

void processCameraAngle() {
	if (mouseStatus[GLFW_MOUSE_BUTTON_LEFT]) {
		double xoffset = (xNewPos - xCurPos) * sensitivity;
		double yoffset = (yCurPos - yNewPos) * sensitivity;
		yaw += xoffset;
		pitch += yoffset;
		if (pitch >= 45.0f) {
			pitch = 45.0f;
		}
		else if (pitch <= -45.0f) {
			pitch = -45.0f;
		}
	}
	xCurPos = xNewPos;
	yCurPos = yNewPos;
}

void updateCamera() {
	processCameraAngle();
	processKeyInfo();
	processCameraMat();
}

void updateCameraMat() {
	viewMatrix = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
	projectionMatrix = glm::perspective(glm::radians(45.0f), (float)wWidth / (float)wHeight, 0.1f, 500.0f);

	// 将矩阵传递给着色器
	GLint viewLoc = glGetUniformLocation(shaderProgram, "view");
	GLint projectionLoc = glGetUniformLocation(shaderProgram, "projection");
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(viewMatrix));
	glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projectionMatrix));
	// 设置光源位置
	glUniform3f(glGetUniformLocation(shaderProgram, "lightPos"), lightPos * sin(theta), 50.0f, lightPos * cos(theta));

	// 设置观察者位置
	glUniform3f(glGetUniformLocation(shaderProgram, "viewPos"), cameraPos.x, cameraPos.y, cameraPos.z);
}

void updateModelMat(int i) {
	GLint modelLoc = glGetUniformLocation(shaderProgram, "model");
	GLint samplerLoc = glGetUniformLocation(shaderProgram, "sampler");
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelMatrix));
	glUniform1i(samplerLoc, i);
}

void renderTree1() {
	glBindVertexArray(vao[TREE1]);
	glm::mat4 temp(1.0f);
	float deltaX = 0.0f, deltaZ = 0.0f;
	temp = glm::translate(temp, glm::vec3(0.0f, 0.4f, 50.0f));
	for (int i = 0; i < 13; i++) {
		deltaX = 6.0f * (i - 6);
		modelMatrix = glm::translate(temp, glm::vec3(deltaX, 0.0f, 0.0f));
		updateModelMat(TREE1);
		glDrawArrays(GL_TRIANGLES, 0, models[TREE1].getNumVertices());
	}
	temp = glm::translate(glm::mat4(1.0f), glm::vec3(40.0f, 0.4f, 0.0f));
	for (int i = 0; i < 15; i++) {
		if (i >= 3 && i <= 11) {
			continue;
		}
		deltaZ = 6.0f * (i - 7);
		modelMatrix = glm::translate(temp, glm::vec3(0.0f, 0.0f, deltaZ));
		updateModelMat(TREE1);
		glDrawArrays(GL_TRIANGLES, 0, models[TREE1].getNumVertices());
	}
	glBindVertexArray(0);
}

void renderTree2() {
	glBindVertexArray(vao[TREE2]);
	glm::mat4 temp(1.0f);
	float deltaZ = 15.0f;
	temp = glm::translate(temp, glm::vec3(40.0f, 0.2f, 0.0f));
	modelMatrix = glm::translate(temp, glm::vec3(0.0f, 0.2f, deltaZ));
	updateModelMat(TREE2);
	glDrawArrays(GL_TRIANGLES, 0, models[TREE2].getNumVertices());
	modelMatrix = glm::translate(temp, glm::vec3(0.0f, 0.2f, -deltaZ));
	updateModelMat(TREE2);
	glDrawArrays(GL_TRIANGLES, 0, models[TREE2].getNumVertices());
	modelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(-50.0f, 0.2f, 40.0f));
	modelMatrix = glm::scale(modelMatrix, glm::vec3(2.5f, 2.5f, 2.5f));
	updateModelMat(TREE2);
	glDrawArrays(GL_TRIANGLES, 0, models[TREE2].getNumVertices());
	glBindVertexArray(0);
}

void renderSquare() {
	glBindVertexArray(vao[SQUARE]);
	modelMatrix = glm::mat4(1.0f);
	updateModelMat(SQUARE);
	glDrawArrays(GL_TRIANGLES, 0, models[SQUARE].getNumVertices());
	glBindVertexArray(0);
}

void renderLib() {
	glBindVertexArray(vao[LIB]);
	modelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -47.0f));
	updateModelMat(LIB);
	glDrawArrays(GL_TRIANGLES, 0, models[LIB].getNumVertices());
	glBindVertexArray(0);
}

void update() {
	updateCamera();
	updateCameraMat();
}

void render() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glUseProgram(shaderProgram);
	update();
	renderTree1();
	renderTree2();
	renderSquare();
	renderLib();
}

void print() {
	cout << "WASD控制移动方向" << endl;
	cout << "按住鼠标左键旋转镜头朝向" << endl;
	cout << "H控制光点位置" << endl;
	cout << "模型导入需要30秒左右" << endl;

}

int main() {
	print();
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(wWidth, wHeight, "OpenGL", NULL, NULL);
	glfwMakeContextCurrent(window);

	glfwSetFramebufferSizeCallback(window, frameBufferSizeCallBack);

	glfwSetKeyCallback(window, keyCallBack);
	glfwSetMouseButtonCallback(window, mouseCallBack);
	glfwSetCursorPosCallback(window, mouseMoveCallBack);
	
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		cout << "Failed to initialize GLAD" << endl;
		return -1;
	}
	prepareStatus();
	prepareShader();
	prepareTexture();
	prepareModelsAndVAO();

	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();
		render();
		glfwSwapBuffers(window);
	}

	// 释放资源
	glfwTerminate();

	return 0;
}