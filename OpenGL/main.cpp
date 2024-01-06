#include <cstdio>
#include <cassert>
#include <cstdlib>
#include <cstring>
#include <string>
#include <map>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>
#include <GL/glew.h>  
#include <GL/gl.h>   
#include <GLFW/glfw3.h> 
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#define _USE_MATH_DEFINES
#include <math.h>
#include <ft2build.h>
#include FT_FREETYPE_H
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define BUFFER_OFFSET(i) ((char*)NULL + (i))

using namespace std;

struct Vertex
{
	Vertex(GLfloat inX, GLfloat inY, GLfloat inZ) : x(inX), y(inY), z(inZ) { }
	GLfloat x, y, z;
};

struct Texture
{
	Texture(GLfloat inU, GLfloat inV) : u(inU), v(inV) { }
	GLfloat u, v;
};

struct Normal
{
	Normal(GLfloat inX, GLfloat inY, GLfloat inZ) : x(inX), y(inY), z(inZ) { }
	GLfloat x, y, z;
};

struct Face
{
	Face(int v[], int t[], int n[]) {
		vIndex[0] = v[0];
		vIndex[1] = v[1];
		vIndex[2] = v[2];
		tIndex[0] = t[0];
		tIndex[1] = t[1];
		tIndex[2] = t[2];
		nIndex[0] = n[0];
		nIndex[1] = n[1];
		nIndex[2] = n[2];
	}
	GLuint vIndex[3], tIndex[3], nIndex[3];
};

struct MeshData
{
	vector<Vertex> gVertices;
	vector<Texture> gTextures;
	vector<Normal> gNormals;
	vector<Face> gFaces;
	GLuint gVertexAttribBuffer, gIndexBuffer;
	int gVertexDataSizeInBytes, gNormalDataSizeInBytes;

	MeshData(vector<Vertex> vertices, vector<Texture> textures, vector<Normal> normals, vector<Face>faces) {
		gVertices = vertices;
		gTextures = textures;
		gNormals = normals;
		gFaces = faces;
	}
};

struct Character {
	GLuint textureID;   // ID handle of the glyph texture
	glm::ivec2 size;    // Size of glyph
	glm::ivec2 bearing;  // Offset from baseline to left/top of glyph
	GLuint advance;    // Horizontal offset to advance to next glyph
};

GLuint gProgram[5];
int gWidth = 800, gHeight = 800;
float corrZ = 0.f;
float bunnyX = 0.f, bunnyY = 0.f, bunnyZ = -3.f;
int score = 0;
double speed = 0.1; // TODO: set speed for 60 FPS
bool isGameActive = true;
bool isAPressed = false, isDPressed = false, isRPressed = false;
int yellowIndex = -1;
int hitedCubeIndex = -1;

vector<MeshData> meshDataList;

glm::vec3 colorYellow(1.0, 0.9, 0.1);
glm::vec3 colorRed(1.0, 0.1, 0.1);

/// Holds all state information relevant to a character as loaded using FreeType
map<GLchar, Character> Characters;
GLuint textureID;
GLuint gTextVBO;
GLint gInVertexLoc, gInNormalLoc;

bool ParseObj(const string& fileName)
{
	vector<Vertex> gVertices;
	vector<Texture> gTextures;
	vector<Normal> gNormals;
	vector<Face> gFaces;

	fstream myfile;

	// Open the input 
	myfile.open(fileName.c_str(), std::ios::in);

	if (myfile.is_open())
	{
		string curLine;

		while (getline(myfile, curLine))
		{
			stringstream str(curLine);
			GLfloat c1, c2, c3;
			GLuint index[9];
			string tmp;

			if (curLine.length() >= 2)
			{
				if (curLine[0] == '#') // comment
				{
					continue;
				}
				else if (curLine[0] == 'v')
				{
					if (curLine[1] == 't') // texture
					{
						str >> tmp; // consume "vt"
						str >> c1 >> c2;
						gTextures.push_back(Texture(c1, c2));
					}
					else if (curLine[1] == 'n') // normal
					{
						str >> tmp; // consume "vn"
						str >> c1 >> c2 >> c3;
						gNormals.push_back(Normal(c1, c2, c3));
					}
					else // vertex
					{
						str >> tmp; // consume "v"
						str >> c1 >> c2 >> c3;
						gVertices.push_back(Vertex(c1, c2, c3));
					}
				}
				else if (curLine[0] == 'f') // face
				{
					str >> tmp; // consume "f"
					char c;
					int vIndex[3], nIndex[3], tIndex[3];
					str >> vIndex[0]; str >> c >> c; // consume "//"
					str >> nIndex[0];
					str >> vIndex[1]; str >> c >> c; // consume "//"
					str >> nIndex[1];
					str >> vIndex[2]; str >> c >> c; // consume "//"
					str >> nIndex[2];

					assert(vIndex[0] == nIndex[0] &&
						vIndex[1] == nIndex[1] &&
						vIndex[2] == nIndex[2]); // a limitation for now

					// make indices start from 0
					for (int c = 0; c < 3; ++c)
					{
						vIndex[c] -= 1;
						nIndex[c] -= 1;
						tIndex[c] -= 1;
					}

					gFaces.push_back(Face(vIndex, tIndex, nIndex));
				}
				else
				{
					cout << "Ignoring unidentified line in obj file: " << curLine << endl;
				}
			}

			//data += curLine;
			if (!myfile.eof())
			{
				//data += "\n";
			}
		}

		myfile.close();
	}
	else
	{
		return false;
	}

	/*
	for (int i = 0; i < gVertices.size(); ++i)
	{
		Vector3 n;

		for (int j = 0; j < gFaces.size(); ++j)
		{
			for (int k = 0; k < 3; ++k)
			{
				if (gFaces[j].vIndex[k] == i)
				{
					// face j contains vertex i
					Vector3 a(gVertices[gFaces[j].vIndex[0]].x,
							  gVertices[gFaces[j].vIndex[0]].y,
							  gVertices[gFaces[j].vIndex[0]].z);

					Vector3 b(gVertices[gFaces[j].vIndex[1]].x,
							  gVertices[gFaces[j].vIndex[1]].y,
							  gVertices[gFaces[j].vIndex[1]].z);

					Vector3 c(gVertices[gFaces[j].vIndex[2]].x,
							  gVertices[gFaces[j].vIndex[2]].y,
							  gVertices[gFaces[j].vIndex[2]].z);

					Vector3 ab = b - a;
					Vector3 ac = c - a;
					Vector3 normalFromThisFace = (ab.cross(ac)).getNormalized();
					n += normalFromThisFace;
				}

			}
		}

		n.normalize();

		gNormals.push_back(Normal(n.x, n.y, n.z));
	}
	*/

	assert(gVertices.size() == gNormals.size());

	meshDataList.push_back(MeshData(gVertices, gTextures, gNormals, gFaces));

	return true;
}

bool ReadDataFromFile(const string& fileName, string& data)
{
	///< [in]  Name of the shader file
	///< [out] The contents of the file
	fstream myfile;

	// Open the input 
	myfile.open(fileName.c_str(), std::ios::in);

	if (myfile.is_open())
	{
		string curLine;

		while (getline(myfile, curLine))
		{
			data += curLine;
			if (!myfile.eof())
			{
				data += "\n";
			}
		}

		myfile.close();
	}
	else
	{
		return false;
	}

	return true;
}

void createVS(GLuint& program, const string& filename)
{
	string shaderSource;

	if (!ReadDataFromFile(filename, shaderSource))
	{
		cout << "Cannot find file name: " + filename << endl;
		exit(-1);
	}

	GLint length = shaderSource.length();
	const GLchar* shader = (const GLchar*)shaderSource.c_str();

	GLuint vs = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vs, 1, &shader, &length);
	glCompileShader(vs);

	char output[1024] = { 0 };
	glGetShaderInfoLog(vs, 1024, &length, output);
	printf("VS compile log: %s\n", output);

	glAttachShader(program, vs);
}

void createFS(GLuint& program, const string& filename)
{
	string shaderSource;

	if (!ReadDataFromFile(filename, shaderSource))
	{
		cout << "Cannot find file name: " + filename << endl;
		exit(-1);
	}

	GLint length = shaderSource.length();
	const GLchar* shader = (const GLchar*)shaderSource.c_str();

	GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fs, 1, &shader, &length);
	glCompileShader(fs);

	char output[1024] = { 0 };
	glGetShaderInfoLog(fs, 1024, &length, output);
	printf("FS compile log: %s\n", output);

	glAttachShader(program, fs);
}

void initBackground()
{
	int  width, height, comp;
	unsigned char* data = stbi_load("sky.jpg", &width, &height, &comp, 0);

	glUseProgram(gProgram[0]);

	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);

	// Set texture wrapping and filtering options if needed
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// Generate the texture from the loaded image data
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);

	// Generate mipmaps for better quality
	glGenerateMipmap(GL_TEXTURE_2D);

	// Free the loaded image data, as it is now stored in OpenGL texture
	stbi_image_free(data);
}

void initShaders()
{
	gProgram[0] = glCreateProgram();
	gProgram[1] = glCreateProgram();
	gProgram[2] = glCreateProgram();
	gProgram[3] = glCreateProgram();
	gProgram[4] = glCreateProgram();

	createVS(gProgram[0], "vert_sky.glsl");
	createFS(gProgram[0], "frag_sky.glsl");

	createVS(gProgram[1], "vert_bunny.glsl");
	createFS(gProgram[1], "frag_bunny.glsl");

	createVS(gProgram[2], "vert_cube.glsl");
	createFS(gProgram[2], "frag_cube.glsl");

	createVS(gProgram[3], "vert_quad.glsl");
	createFS(gProgram[3], "frag_quad.glsl");

	createVS(gProgram[4], "vert_text.glsl");
	createFS(gProgram[4], "frag_text.glsl");


	glBindAttribLocation(gProgram[0], 0, "inVertex");
	glBindAttribLocation(gProgram[0], 1, "inNormal");
	glBindAttribLocation(gProgram[1], 0, "inVertex");
	glBindAttribLocation(gProgram[1], 1, "inNormal");
	glBindAttribLocation(gProgram[2], 0, "inVertex");
	glBindAttribLocation(gProgram[2], 1, "inNormal");
	glBindAttribLocation(gProgram[3], 0, "inVertex");
	glBindAttribLocation(gProgram[3], 1, "inNormal");
	glBindAttribLocation(gProgram[4], 2, "vertex");

	glLinkProgram(gProgram[0]);
	glLinkProgram(gProgram[1]);
	glLinkProgram(gProgram[2]);
	glLinkProgram(gProgram[3]);
	glLinkProgram(gProgram[4]);

	glUseProgram(gProgram[0]);
}

void initVBO(MeshData& meshData)
{
	// Initialization Vertex Buffer Object

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	assert(glGetError() == GL_NONE);

	// Generate Buffer
	glGenBuffers(1, &meshData.gVertexAttribBuffer);
	glGenBuffers(1, &meshData.gIndexBuffer);

	assert(meshData.gVertexAttribBuffer > 0 && meshData.gIndexBuffer > 0);

	meshData.gVertexDataSizeInBytes = meshData.gVertices.size() * 3 * sizeof(GLfloat);
	meshData.gNormalDataSizeInBytes = meshData.gNormals.size() * 3 * sizeof(GLfloat);

	int indexDataSizeInBytes = meshData.gFaces.size() * 3 * sizeof(GLuint);
	GLfloat* vertexData = new GLfloat[meshData.gVertices.size() * 3];
	GLfloat* normalData = new GLfloat[meshData.gNormals.size() * 3];
	GLuint* indexData = new GLuint[meshData.gFaces.size() * 3];

	float minX = 1e6, maxX = -1e6;
	float minY = 1e6, maxY = -1e6;
	float minZ = 1e6, maxZ = -1e6;

	for (int i = 0; i < meshData.gVertices.size(); ++i)
	{
		vertexData[3 * i] = meshData.gVertices[i].x;
		vertexData[3 * i + 1] = meshData.gVertices[i].y;
		vertexData[3 * i + 2] = meshData.gVertices[i].z;

		minX = min(minX, meshData.gVertices[i].x);
		maxX = max(maxX, meshData.gVertices[i].x);
		minY = min(minY, meshData.gVertices[i].y);
		maxY = max(maxY, meshData.gVertices[i].y);
		minZ = min(minZ, meshData.gVertices[i].z);
		maxZ = max(maxZ, meshData.gVertices[i].z);
	}

	/*std::cout << "minX = " << minX << std::endl;
	std::cout << "maxX = " << maxX << std::endl;
	std::cout << "minY = " << minY << std::endl;
	std::cout << "maxY = " << maxY << std::endl;
	std::cout << "minZ = " << minZ << std::endl;
	std::cout << "maxZ = " << maxZ << std::endl;*/

	for (int i = 0; i < meshData.gNormals.size(); ++i)
	{
		normalData[3 * i] = meshData.gNormals[i].x;
		normalData[3 * i + 1] = meshData.gNormals[i].y;
		normalData[3 * i + 2] = meshData.gNormals[i].z;
	}

	for (int i = 0; i < meshData.gFaces.size(); ++i)
	{
		indexData[3 * i] = meshData.gFaces[i].vIndex[0];
		indexData[3 * i + 1] = meshData.gFaces[i].vIndex[1];
		indexData[3 * i + 2] = meshData.gFaces[i].vIndex[2];
	}

	// Copy Data to GPU Memory
	glBindBuffer(GL_ARRAY_BUFFER, meshData.gVertexAttribBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, meshData.gIndexBuffer);
	glBufferData(GL_ARRAY_BUFFER, meshData.gVertexDataSizeInBytes + meshData.gNormalDataSizeInBytes, 0, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, meshData.gVertexDataSizeInBytes, vertexData);
	glBufferSubData(GL_ARRAY_BUFFER, meshData.gVertexDataSizeInBytes, meshData.gNormalDataSizeInBytes, normalData);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexDataSizeInBytes, indexData, GL_STATIC_DRAW);

	// done copying; can free now
	delete[] vertexData;
	delete[] normalData;
	delete[] indexData;

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(meshData.gVertexDataSizeInBytes));

}

void initFonts(int windowWidth, int windowHeight)
{
	// Set OpenGL options
	glEnable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glm::mat4 projection = glm::ortho(0.0f, static_cast<GLfloat>(windowWidth), 0.0f, static_cast<GLfloat>(windowHeight));
	glUseProgram(gProgram[4]);
	glUniformMatrix4fv(glGetUniformLocation(gProgram[4], "projection"), 1, GL_FALSE, glm::value_ptr(projection));

	// FreeType
	FT_Library ft;
	// All functions return a value different than 0 whenever an error occurred
	if (FT_Init_FreeType(&ft))
	{
		std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
	}

	// Load font as face
	FT_Face face;
	// TODO : 
	// if (FT_New_Face(ft, "/usr/share/fonts/truetype/liberation/LiberationSerif-Italic.ttf", 0, &face))
	if (FT_New_Face(ft, "C:/Users/QP/Desktop/OpenGL/libraries/freetype-windows-binaries-master/freetype-windows-binaries-master/fonts/arial.ttf", 0, &face))
	{
		std::cout << "ERROR::FREETYPE: Failed to load font" << std::endl;
	}

	// Set size to load glyphs as
	FT_Set_Pixel_Sizes(face, 0, 48);

	// Disable byte-alignment restriction
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	// Load first 128 characters of ASCII set
	for (GLubyte c = 0; c < 128; c++)
	{
		// Load character glyph 
		if (FT_Load_Char(face, c, FT_LOAD_RENDER))
		{
			std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
			continue;
		}
		// Generate texture
		GLuint texture;
		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);
		glTexImage2D(
			GL_TEXTURE_2D,
			0,
			GL_RED,
			face->glyph->bitmap.width,
			face->glyph->bitmap.rows,
			0,
			GL_RED,
			GL_UNSIGNED_BYTE,
			face->glyph->bitmap.buffer
		);
		// Set texture options
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		// Now store character for later use
		Character character = {
			texture,
			glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
			glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
			face->glyph->advance.x
		};
		Characters.insert(std::pair<GLchar, Character>(c, character));
	}

	glBindTexture(GL_TEXTURE_2D, 0);
	// Destroy FreeType once we're finished
	FT_Done_Face(face);
	FT_Done_FreeType(ft);

	//
	// Configure VBO for texture quads
	//
	glGenBuffers(1, &gTextVBO);
	glBindBuffer(GL_ARRAY_BUFFER, gTextVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 6 * 4, NULL, GL_DYNAMIC_DRAW);

	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), 0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void init()
{
	ParseObj("bunny.obj");
	ParseObj("cube.obj");
	ParseObj("cube.obj");
	ParseObj("cube.obj");
	ParseObj("quad.obj");
	glEnable(GL_DEPTH_TEST);
	initShaders();
	initFonts(gWidth, gHeight);
	initBackground();
	for (int i = 0; i < meshDataList.size(); i++)
	{
		initVBO(meshDataList[i]);
	}
}

void drawBackground()
{
	glUseProgram(gProgram[0]);

	glBindTexture(GL_TEXTURE_2D, textureID);

	glBegin(GL_QUADS);
	glTexCoord2f(0.0, 1.0); // bottom-left
	glVertex3f(-1.0, -1.0, 0.0);
	glTexCoord2f(1.0, 1.0); // bottom-right
	glVertex3f(1.0, -1.0, 0.0);
	glTexCoord2f(1.0, 0.0); // top-right
	glVertex3f(1.0, 1.0, 0.0);
	glTexCoord2f(0.0, 0.0); // top-left
	glVertex3f(-1.0, 1.0, 0.0);
	glEnd();
}

void drawModel(MeshData& meshData)
{
	glBindBuffer(GL_ARRAY_BUFFER, meshData.gVertexAttribBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, meshData.gIndexBuffer);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(meshData.gVertexDataSizeInBytes));

	glDrawElements(GL_TRIANGLES, meshData.gFaces.size() * 3, GL_UNSIGNED_INT, 0);
}

void renderText(const std::string& text, GLfloat x, GLfloat y, GLfloat scale)
{
	glm::vec3 color = isGameActive ? colorYellow : colorRed;

	// Activate corresponding render state	
	glUseProgram(gProgram[4]);
	glUniform3f(glGetUniformLocation(gProgram[4], "textColor"), color.x, color.y, color.z);
	glActiveTexture(GL_TEXTURE0);

	// Iterate through all characters
	std::string::const_iterator c;
	for (c = text.begin(); c != text.end(); c++)
	{
		Character ch = Characters[*c];

		GLfloat xpos = x + ch.bearing.x * scale + 5;
		GLfloat ypos = y - (ch.size.y - ch.bearing.y) * scale + gHeight - 30;

		GLfloat w = ch.size.x * scale;
		GLfloat h = ch.size.y * scale;

		// Update VBO for each character
		GLfloat vertices[6][4] = {
			{ xpos,     ypos + h,   0.0, 0.0 },
			{ xpos,     ypos,       0.0, 1.0 },
			{ xpos + w, ypos,       1.0, 1.0 },

			{ xpos,     ypos + h,   0.0, 0.0 },
			{ xpos + w, ypos,       1.0, 1.0 },
			{ xpos + w, ypos + h,   1.0, 0.0 }
		};

		// Render glyph texture over quad
		glBindTexture(GL_TEXTURE_2D, ch.textureID);

		// Update content of VBO memory
		glBindBuffer(GL_ARRAY_BUFFER, gTextVBO);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices); // Be sure to use glBufferSubData and not glBufferData

		//glBindBuffer(GL_ARRAY_BUFFER, 0);

		// Render quad
		glDrawArrays(GL_TRIANGLES, 0, 6);
		// Now advance cursors for next glyph (note that advance is number of 1/64 pixels)

		x += (ch.advance >> 6) * scale; // Bitshift by 6 to get value in pixels (2^6 = 64 (divide amount of 1/64th pixels by 64 to get amount of pixels))
	}

	glBindTexture(GL_TEXTURE_2D, 0);
}

bool checkCollisionWithBunny(float cubeX, float cubeZ)
{
	double bunnyWidth = 3 * 0.2;
	double bunnyDepth = 2.4 * 0.2;
	double cubeWidth = 2 * 0.4;
	double cubeDepth = 2 * 0.4;

	double bunnyMinZ = bunnyZ - bunnyDepth / 2;

	double cubeMinX = cubeX - cubeWidth / 2;
	double cubeMaxX = cubeX + cubeWidth / 2;
	double cubeMinZ = cubeZ - cubeDepth / 2;
	double cubeMaxZ = cubeZ + cubeDepth / 2;

	if ((bunnyX <= cubeMaxX && bunnyX >= cubeMinX) && (bunnyMinZ >= cubeMinZ && bunnyMinZ <= cubeMaxZ))
	{
		printf("Collision detected!\n");
		return true;
	}
	return false;
}

void display()
{
	glClearColor(0, 0, 0, 1);
	glClearDepth(1.0f);
	glClearStencil(0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	static float time = 0;
	static bool isRotationActive = false;
	static float bunnyRotationAngle = 0.0f;

	float bunnyJumpHeight = 1.0f * sin(time); // Use sine function for smoother jump

	glm::mat4 T;
	glm::mat4 S;
	glm::mat4 R;
	glm::mat4 modelMat;
	glm::mat4 perspMat;
	glm::mat4 modelMatGround;
	perspMat = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 200.0f);

	corrZ -= speed * 2;
	if (isGameActive)
	{
		// TODO set speed for 60 fps
		speed += 0.00005;
		score += speed * 10;
	}

	if (corrZ <= -30)
	{
		corrZ = fmod(corrZ, 30.0);
		yellowIndex = -1;
		hitedCubeIndex = -1;
	}

	if (yellowIndex == -1) {
		yellowIndex = rand() % 3;
	}

	// Display background
	glUseProgram(gProgram[0]);
	drawBackground();

	// Display cubes
	glUseProgram(gProgram[2]);
	for (int i = 0; i < 3; i++) {
		double cubeX, cubeZ = -30.f - corrZ;
		if (i == 0)
			cubeX = -2.5;
		else if (i == 1)
			cubeX = 0;
		else if (i == 2)
			cubeX = 2.5;

		if (isGameActive && !isRotationActive && checkCollisionWithBunny(cubeX, cubeZ)) {
			if (yellowIndex == i) {
				score += 1000;
				isRotationActive = true;
			}
			else {
				speed = 0;
				isGameActive = false;
			}
			hitedCubeIndex = i;
		}

		T = glm::translate(glm::mat4(1.f), glm::vec3(cubeX, -1, cubeZ));
		S = glm::scale(glm::mat4(1.0), glm::vec3(0.4, 0.8, 0.4));
		modelMat = T * S;

		if (hitedCubeIndex != i) {
			GLint kdLocation = glGetUniformLocation(gProgram[2], "kd");
			if (yellowIndex == i)
				glUniform3fv(kdLocation, 1, glm::value_ptr(colorYellow));
			else
				glUniform3fv(kdLocation, 1, glm::value_ptr(colorRed));

			glUniformMatrix4fv(glGetUniformLocation(gProgram[2], "modelingMat2"), 1, GL_FALSE, glm::value_ptr(modelMat));
			glUniformMatrix4fv(glGetUniformLocation(gProgram[2], "modelingMatInvTr2"), 1, GL_FALSE, glm::value_ptr(glm::transpose(glm::inverse(modelMat))));
			glUniformMatrix4fv(glGetUniformLocation(gProgram[2], "perspectiveMat2"), 1, GL_FALSE, glm::value_ptr(perspMat));

			drawModel(meshDataList[i + 1]);
		}
	}

	// Display bunny
	glUseProgram(gProgram[1]);
	T = glm::translate(glm::mat4(1.f), glm::vec3(bunnyX, bunnyJumpHeight - 2.f, -3.0));
	S = glm::scale(glm::mat4(1.0), glm::vec3(0.2, 0.25, 0.2));
	R = glm::rotate<float>(glm::mat4(1.0), (270. / 180.) * M_PI + glm::radians(bunnyRotationAngle), glm::vec3(0.0, 1.0, 0.0));
	if (!isGameActive) {
		glm::mat4 R_z = glm::rotate<float>(glm::mat4(1.0), (-90. / 180.) * M_PI, glm::vec3(1.0, 0.0, 0.0));
		R = R * R_z;
	}
	modelMat = T * S * R;

	// Reset time to prevent overflow
	time += speed;
	if (isRotationActive)
		bunnyRotationAngle += speed * 100;
	if (time > M_PI) {
		time = 0.0;
	}
	if (bunnyRotationAngle >= 360.0f) {
		bunnyRotationAngle = 0.0f;
		isRotationActive = false;
	}
	glUniformMatrix4fv(glGetUniformLocation(gProgram[1], "modelingMat"), 1, GL_FALSE, glm::value_ptr(modelMat));
	glUniformMatrix4fv(glGetUniformLocation(gProgram[1], "modelingMatInvTr"), 1, GL_FALSE, glm::value_ptr(glm::transpose(glm::inverse(modelMat))));
	glUniformMatrix4fv(glGetUniformLocation(gProgram[1], "perspectiveMat"), 1, GL_FALSE, glm::value_ptr(perspMat));

	drawModel(meshDataList[0]);

	// Display ground
	glUseProgram(gProgram[3]);
	T = glm::translate(glm::mat4(1.f), glm::vec3(0, -2.0, corrZ));
	S = glm::scale(glm::mat4(1.0), glm::vec3(4, 1, 100));
	R = glm::rotate<float>(glm::mat4(1.0), (-90 / 180.) * M_PI, glm::vec3(1.0, 0.0, 0.0));
	modelMatGround = T * S * R;
	glUniformMatrix4fv(glGetUniformLocation(gProgram[3], "modelingMat"), 1, GL_FALSE, glm::value_ptr(modelMatGround));
	glUniformMatrix4fv(glGetUniformLocation(gProgram[3], "modelingMatInvTr"), 1, GL_FALSE, glm::value_ptr(glm::transpose(glm::inverse(modelMatGround))));
	glUniformMatrix4fv(glGetUniformLocation(gProgram[3], "perspectiveMat"), 1, GL_FALSE, glm::value_ptr(perspMat));

	// Set offsetX and offsetZ and scales for ground
	float offsetX = 3.9;
	float offsetZ = corrZ;
	float scaleX = 0.63;
	float scaleZ = 0.2;

	int offsetXLocation = glGetUniformLocation(gProgram[3], "offsetX");
	int offsetZLocation = glGetUniformLocation(gProgram[3], "offsetZ");
	int scaleXLocation = glGetUniformLocation(gProgram[3], "scaleX");
	int scaleZLocation = glGetUniformLocation(gProgram[3], "scaleZ");
	glUniform1f(offsetXLocation, offsetX);
	glUniform1f(offsetZLocation, offsetZ);
	glUniform1f(scaleXLocation, scaleX);
	glUniform1f(scaleZLocation, scaleZ);

	drawModel(meshDataList[4]);

	// Display Text
	renderText("Score: " + to_string(score), 1, 1, 0.5);
}

void reshape(GLFWwindow* window, int w, int h)
{
	w = w < 1 ? 1 : w;
	h = h < 1 ? 1 : h;

	gWidth = w;
	gHeight = h;

	glViewport(0, 0, w, h);
}

void update()
{
	if (isAPressed)
	{
		if (isGameActive && bunnyX != -2.5) {
			cout << "A pressed" << endl;
			bunnyX -= 1.25;
			glUseProgram(gProgram[1]);
		}
	}
	if (isDPressed)
	{
		if (isGameActive && bunnyX != 2.5) {
			cout << "D pressed" << endl;
			bunnyX += 1.25;
			glUseProgram(gProgram[1]);
		}
	}
	if (isRPressed)
	{
		cout << "R pressed" << endl;
		score = 0;
		// TODO set speed for 60 FPS
		speed = 0.1;
		isGameActive = true;
		corrZ = 0;
		yellowIndex = -1;
		hitedCubeIndex = -1;
	}
}

void keyboard(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (action == GLFW_PRESS)
	{
		if (key == GLFW_KEY_ESCAPE)
			glfwSetWindowShouldClose(window, GL_TRUE);
		else if (key == GLFW_KEY_A)
			isAPressed = true;
		else if (key == GLFW_KEY_D)
			isDPressed = true;
		else if (key == GLFW_KEY_R)
			isRPressed = true;
	}
	if (action == GLFW_RELEASE)
	{
		if (key == GLFW_KEY_A)
			isAPressed = false;
		else if (key == GLFW_KEY_D)
			isDPressed = false;
		else if (key == GLFW_KEY_R)
			isRPressed = false;
	}
	update();
}

void mainLoop(GLFWwindow* window)
{
	while (!glfwWindowShouldClose(window))
	{
		display();
		// Handle double buffering 
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
}

int main(int argc, char** argv)   // Create Main Function For Bringing It All Together
{
	GLFWwindow* window;
	if (!glfwInit())
	{
		exit(-1);
	}
	// That indicates OpenGl Version 2.1
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
	//glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);
	//glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

	window = glfwCreateWindow(gWidth, gHeight, "Bunny Game", NULL, NULL);
	if (!window)
	{
		glfwTerminate();
		exit(-1);
	}

	glfwMakeContextCurrent(window);

	//TODO: To avoid Tearing adjust VSync
	glfwSwapInterval(280 / 60);

	// Initialize GLEW to setup the OpenGL Function pointers
	if (GLEW_OK != glewInit())
	{
		std::cout << "Failed to initialize GLEW" << std::endl;
		return EXIT_FAILURE;
	}

	char rendererInfo[512] = { 0 };
	strcpy_s(rendererInfo, (const char*)glGetString(GL_RENDERER));
	strcat_s(rendererInfo, " - ");
	strcat_s(rendererInfo, (const char*)glGetString(GL_VERSION));
	glfwSetWindowTitle(window, rendererInfo);

	init();
	reshape(window, gWidth, gHeight);

	glfwSetKeyCallback(window, keyboard);
	glfwSetWindowSizeCallback(window, reshape);

	mainLoop(window); // this does not return unless the window is closed

	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}

