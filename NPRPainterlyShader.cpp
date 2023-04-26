#include <stdio.h>
#include <stdlib.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
GLFWwindow* window;

#include <string>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>

#include "ref/controls.hpp"
#include "ref/tangentspace.hpp"
#include "ref/objloader.hpp"
#include "ref/shader.hpp"
#include "ref/texture.hpp"
#include "ref/vboindexer.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include "ref/stb_image.h"

using namespace glm;

void initializeScreenspaceQuadGrid(int grid_dimensions,
	std::vector<glm::vec3>& quad_grid_verts, std::vector<glm::vec2>& quad_uvs,
	std::vector<unsigned short>& quad_indices,
	std::vector<float>& brush_widths, std::vector<float>& brush_lengths,
	std::vector<float>& brush_rotations, std::vector<glm::vec3>& color_variance) {
	// std::vector<glm::vec3>& quad_color_variance) {
	const int STRIDE = 4;
	const float delta = 2.0 / grid_dimensions;
	const float BRUSH_SIZE = delta;
	const float BRUSH_SIZE_VARIANCE = delta * 6.0; // Best with delta / 3.0
	const float ANGLE = 0.0; // Degrees // Best with -30
	const float ANGLE_VARIANCE = 100.0; // Degrees // Best with 10
	const float COLOR_VARIANCE = 0.1; // Best with 0.05

	for (int x = 0; x < grid_dimensions; x++) {
		float deltaX = x * delta;
		for (int y = 0; y < grid_dimensions; y++) {
			float deltaY = y * delta;
			quad_grid_verts.push_back(glm::vec3(-1.0 + deltaX, -1.0 + deltaY, 0));
			quad_grid_verts.push_back(glm::vec3(-1.0 + deltaX + BRUSH_SIZE, -1.0 + deltaY, 0));
			quad_grid_verts.push_back(glm::vec3(-1.0 + deltaX, -1.0 + deltaY + BRUSH_SIZE, 0));
			quad_grid_verts.push_back(glm::vec3(-1.0 + deltaX + BRUSH_SIZE, -1.0 + deltaY + BRUSH_SIZE, 0));

			quad_uvs.push_back(glm::vec2(0, 0));
			quad_uvs.push_back(glm::vec2(1, 0));
			quad_uvs.push_back(glm::vec2(0, 1));
			quad_uvs.push_back(glm::vec2(1, 1));

			quad_indices.push_back(x * STRIDE + y * STRIDE * grid_dimensions);
			quad_indices.push_back(x * STRIDE + y * STRIDE * grid_dimensions + 1);
			quad_indices.push_back(x * STRIDE + y * STRIDE * grid_dimensions + 2);
			quad_indices.push_back(x * STRIDE + y * STRIDE * grid_dimensions + 2);
			quad_indices.push_back(x * STRIDE + y * STRIDE * grid_dimensions + 1);
			quad_indices.push_back(x * STRIDE + y * STRIDE * grid_dimensions + 3);

			float width = BRUSH_SIZE + ((rand() % 1000) / 1000.0) * BRUSH_SIZE_VARIANCE - BRUSH_SIZE_VARIANCE / 2.0;
			brush_widths.push_back(width);
			brush_widths.push_back(width);
			brush_widths.push_back(width);
			brush_widths.push_back(width);

			float length = BRUSH_SIZE * 2.2 + ((rand() % 1000) / 1000.0) * BRUSH_SIZE_VARIANCE - BRUSH_SIZE_VARIANCE / 2.0;
			brush_lengths.push_back(length);
			brush_lengths.push_back(length);
			brush_lengths.push_back(length);
			brush_lengths.push_back(length);

			float rotation = (3.14 / 180) * (ANGLE + ((rand() % 1000) / 1000.0) * ANGLE_VARIANCE); // In radians
			brush_rotations.push_back(rotation);
			brush_rotations.push_back(rotation);
			brush_rotations.push_back(rotation);
			brush_rotations.push_back(rotation);

			glm::vec3 color_var = glm::vec3(
				((rand() % 1000) / 1000.0) * COLOR_VARIANCE - COLOR_VARIANCE / 2.0,
				((rand() % 1000) / 1000.0) * COLOR_VARIANCE - COLOR_VARIANCE / 2.0,
				((rand() % 1000) / 1000.0) * COLOR_VARIANCE - COLOR_VARIANCE / 2.0);
			color_variance.push_back(color_var);
			color_variance.push_back(color_var);
			color_variance.push_back(color_var);
			color_variance.push_back(color_var);
		}
	}
}


GLuint loadRGBTexture(char* texture_filepath) {
	// Load in file in grayscale
	int texture_width, texture_height, components_per_pixel;
	unsigned char* brush_texture_data = stbi_load(texture_filepath, &texture_width, &texture_height,
		&components_per_pixel, STBI_rgb);

	// Setup texture with Opengl
	GLuint textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texture_width, texture_height, 0, GL_RGB, GL_UNSIGNED_BYTE, brush_texture_data);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	return textureID;
}

GLuint loadGrayscaleTexture(char* texture_filepath) {
	// Load in file in grayscale
	int texture_width, texture_height, components_per_pixel;
	unsigned char* brush_texture_data = stbi_load(texture_filepath, &texture_width, &texture_height,
		&components_per_pixel, STBI_grey);

	// Setup texture with Opengl
	GLuint textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, texture_width, texture_height, 0, GL_RED, GL_UNSIGNED_BYTE, brush_texture_data);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	return textureID;
}

int main(void)
{
	// Initialise GLFW
	if (!glfwInit())
	{
		fprintf(stderr, "Failed to initialize GLFW\n");
		getchar();
		return -1;
	}

	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Open a window and create its OpenGL context
	window = glfwCreateWindow(1024 * 2, 768, "Playground", NULL, NULL);
	if (window == NULL) {
		fprintf(stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n");
		getchar();
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	// Initialize GLEW
	glewExperimental = true;
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		getchar();
		glfwTerminate();
		return -1;
	}

	// Ensure we can capture the escape key being pressed below
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
	// Hide the mouse and enable unlimited mouvement
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	// Set the mouse at the center of the screen
	glfwPollEvents();

	// Pale white background
	glm::vec4 BACKGROUND_COLOR = vec4(227.0 / 255.0, 218.0 / 255.0, 193.0 / 255.0, 0.0f);
	glClearColor(BACKGROUND_COLOR.r, BACKGROUND_COLOR.g, BACKGROUND_COLOR.b, BACKGROUND_COLOR.a);
	int windowWidth, windowHeight;
	glfwGetWindowSize(window, &windowWidth, &windowHeight);
	// Enable depth test
	glEnable(GL_DEPTH_TEST);
	// Accept fragment if it closer to the camera than the former one
	glDepthFunc(GL_LESS);
	glEnable(GL_CULL_FACE);

	GLuint VertexArrayID;
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);

	GLuint simpleRenderProgramID = LoadShaders(
		"SimpleVertexShader.vertexshader",
		"SimpleFragmentShader.fragmentshader");

	// Get a handle for our "MVP" uniform
	GLuint MatrixID = glGetUniformLocation(simpleRenderProgramID, "MVP");
	GLuint ViewMatrixID = glGetUniformLocation(simpleRenderProgramID, "V");
	GLuint ModelMatrixID = glGetUniformLocation(simpleRenderProgramID, "M");
	GLuint ModelView3x3MatrixID = glGetUniformLocation(simpleRenderProgramID, "MV3x3");

	// Load the texture
	GLuint DiffuseTexture = loadRGBTexture("assets/flat-red.jpg");
	GLuint DiffuseTextureID = glGetUniformLocation(simpleRenderProgramID, "myTextureSampler");

	// Get a handle for our "LightPosition" uniform
	glUseProgram(simpleRenderProgramID);
	GLuint LightID = glGetUniformLocation(simpleRenderProgramID, "LightPosition_worldspace");


	// Read our .obj file
	std::vector<glm::vec3> obj_vertices;
	std::vector<glm::vec2> obj_uvs;
	std::vector<glm::vec3> obj_normals;
	bool res = loadOBJ("assets/monkey.obj",
		obj_vertices, obj_uvs, obj_normals);

	std::vector<glm::vec3> obj_tangents;
	std::vector<glm::vec3> obj_bitangents;
	computeTangentBasis(
		obj_vertices, obj_uvs, obj_normals, // input
		obj_tangents, obj_bitangents		// output
	);

	std::vector<unsigned short> indexed_indices;
	std::vector<glm::vec3> indexed_vertices;
	std::vector<glm::vec2> indexed_uvs;
	std::vector<glm::vec3> indexed_normals;
	std::vector<glm::vec3> indexed_tangents;
	std::vector<glm::vec3> indexed_bitangents;
	indexVBO_TBN(obj_vertices, obj_uvs, obj_normals, obj_tangents, obj_bitangents,
		indexed_indices, indexed_vertices, indexed_uvs, indexed_normals, indexed_tangents, indexed_bitangents);

	// Load it into a VBO
	GLuint obj_vertexbuffer;
	glGenBuffers(1, &obj_vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, obj_vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, indexed_vertices.size() * sizeof(glm::vec3), &indexed_vertices[0], GL_STATIC_DRAW);

	GLuint obj_uvbuffer;
	glGenBuffers(1, &obj_uvbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, obj_uvbuffer);
	glBufferData(GL_ARRAY_BUFFER, indexed_uvs.size() * sizeof(glm::vec2), &indexed_uvs[0], GL_STATIC_DRAW);

	GLuint obj_normalbuffer;
	glGenBuffers(1, &obj_normalbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, obj_normalbuffer);
	glBufferData(GL_ARRAY_BUFFER, indexed_normals.size() * sizeof(glm::vec3), &indexed_normals[0], GL_STATIC_DRAW);

	GLuint obj_tangentbuffer;
	glGenBuffers(1, &obj_tangentbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, obj_tangentbuffer);
	glBufferData(GL_ARRAY_BUFFER, indexed_tangents.size() * sizeof(glm::vec2), &indexed_tangents[0], GL_STATIC_DRAW);

	GLuint obj_bitangentbuffer;
	glGenBuffers(1, &obj_bitangentbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, obj_bitangentbuffer);
	glBufferData(GL_ARRAY_BUFFER, indexed_bitangents.size() * sizeof(glm::vec3), &indexed_bitangents[0], GL_STATIC_DRAW);

	// Generate a buffer for the indices as well
	GLuint obj_elementbuffer;
	glGenBuffers(1, &obj_elementbuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, obj_elementbuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexed_indices.size() * sizeof(unsigned short), &indexed_indices[0], GL_STATIC_DRAW);


	//	**************************************	//
	//	**************************************	//
	//		  Post Processing Texturing			//		
	//	**************************************	//
	//	**************************************	//

	GLuint postProcessProgramId = LoadShaders(
		"PostProcessing.vertexshader",
		"PostProcessing.fragmentshader"
	);

	// The framebuffer, which regroups 0, 1, or more textures, and 0 or 1 depth buffer.
	GLuint FramebufferName = 0;
	glGenFramebuffers(1, &FramebufferName);
	glBindFramebuffer(GL_FRAMEBUFFER, FramebufferName);

	// The texture we're going to render to
	GLuint RenderedTexture;
	glGenTextures(1, &RenderedTexture);
	// "Bind" the newly created texture : all future texture functions will modify this texture
	glBindTexture(GL_TEXTURE_2D, RenderedTexture);
	// Give an empty image to OpenGL ( the last "0" )
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, windowWidth / 2, windowHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	GLuint RenderedTextureID = glGetUniformLocation(postProcessProgramId, "rederedTexture");

	GLuint BrushTangentTexture;
	glGenTextures(1, &BrushTangentTexture);
	glBindTexture(GL_TEXTURE_2D, BrushTangentTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, windowWidth / 2, windowHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	GLuint BrushTangentTextureID = glGetUniformLocation(postProcessProgramId, "brushTangentTexture");

	GLuint BrushBitangentTexture;
	glGenTextures(1, &BrushBitangentTexture);
	glBindTexture(GL_TEXTURE_2D, BrushBitangentTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, windowWidth / 2, windowHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	GLuint BrushBitangentTextureID = glGetUniformLocation(postProcessProgramId, "brushBitangentTexture");


	//GLuint BrushTexture = loadGrayscaleTexture("C:\\Users\\lando\\OneDrive\\Documents\\code_projects\\ogl-master\\assets\\FlowerSplochBrush.png");
	GLuint BrushTexture = loadGrayscaleTexture("assets/UnevenBrushStroke.png");
	GLuint BrushTextureID = glGetUniformLocation(postProcessProgramId, "brushTexture");
	// GLuint BrushScaleMatrixID = glGetUniformLocation(postProcessProgramId, "brushScale");
	GLuint BackgroundColorID = glGetUniformLocation(postProcessProgramId, "backgroundColor");

	// The depth buffer
	GLuint depthRenderBuffer;
	glGenRenderbuffers(1, &depthRenderBuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, depthRenderBuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, windowWidth / 2, windowHeight);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthRenderBuffer);

	// Set "renderedTexture" as our colour attachement #0
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, RenderedTexture, 0);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, BrushTangentTexture, 0);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, BrushBitangentTexture, 0);

	// Set the list of draw buffers.
	GLenum DrawBuffers[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
	glDrawBuffers(3, DrawBuffers); // "1" is the size of DrawBuffers

	// Always check that our framebuffer is ok
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		return -1;

	// The fullscreen quad's FBO
	GLuint quad_VertexArrayID;
	glGenVertexArrays(1, &quad_VertexArrayID);
	glBindVertexArray(quad_VertexArrayID);

	// Define a grid of quads to fill the screen, these quads are textured with brush strokes
	std::vector<glm::vec3> g_quad_vertex_buffer_data;
	std::vector<glm::vec2> g_quad_uv_buffer_data;
	std::vector<float> g_brush_widths_data;
	std::vector<float> g_brush_lengths_data;
	std::vector<float> g_brush_rotation_data;
	std::vector<glm::vec3> g_color_variance_data;
	std::vector<unsigned short> g_quad_indices;
	// glm::mat3 brushScaleMat;
	const int QUAD_GRID_SIZE = 50; // Results in a grid SIZE X SIZE  // Best arount 40-60
	initializeScreenspaceQuadGrid(
		QUAD_GRID_SIZE, g_quad_vertex_buffer_data, g_quad_uv_buffer_data,
		g_quad_indices, g_brush_widths_data, g_brush_lengths_data, g_brush_rotation_data, g_color_variance_data);

	GLuint quad_vertexbuffer;
	glGenBuffers(1, &quad_vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, quad_vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, g_quad_vertex_buffer_data.size() * sizeof(glm::vec3), &g_quad_vertex_buffer_data[0], GL_STATIC_DRAW);

	GLuint quad_uvbuffer;
	glGenBuffers(1, &quad_uvbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, quad_uvbuffer);
	glBufferData(GL_ARRAY_BUFFER, g_quad_uv_buffer_data.size() * sizeof(glm::vec2), &g_quad_uv_buffer_data[0], GL_STATIC_DRAW);

	GLuint quad_brushwidths_buffer;
	glGenBuffers(1, &quad_brushwidths_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, quad_brushwidths_buffer);
	glBufferData(GL_ARRAY_BUFFER, g_brush_widths_data.size() * sizeof(float), &g_brush_widths_data[0], GL_STATIC_DRAW);

	GLuint quad_brushlengths_buffer;
	glGenBuffers(1, &quad_brushlengths_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, quad_brushlengths_buffer);
	glBufferData(GL_ARRAY_BUFFER, g_brush_lengths_data.size() * sizeof(float), &g_brush_lengths_data[0], GL_STATIC_DRAW);

	GLuint quad_brushrotation_buffer;
	glGenBuffers(1, &quad_brushrotation_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, quad_brushrotation_buffer);
	glBufferData(GL_ARRAY_BUFFER, g_brush_rotation_data.size() * sizeof(float), &g_brush_rotation_data[0], GL_STATIC_DRAW);

	GLuint quad_colorvariance_buffer;
	glGenBuffers(1, &quad_colorvariance_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, quad_colorvariance_buffer);
	glBufferData(GL_ARRAY_BUFFER, g_color_variance_data.size() * sizeof(glm::vec3), &g_color_variance_data[0], GL_STATIC_DRAW);

	GLuint quad_elementbuffer;
	glGenBuffers(1, &quad_elementbuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, quad_elementbuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, g_quad_indices.size() * sizeof(unsigned short), &g_quad_indices[0], GL_STATIC_DRAW);


	// FPS counter
	double lastTime = glfwGetTime();
	int nbFrames = 0;
	do {
		double currentTime = glfwGetTime();
		nbFrames++;
		if (currentTime - lastTime >= 1.0) {
			printf("%f ms/frame\n", 1000.0 / double(nbFrames));
			nbFrames = 0;
			lastTime += 1.0;
		}


		// Render to our framebuffer
		glBindFramebuffer(GL_FRAMEBUFFER, FramebufferName);
		// Render on the whole framebuffer, complete from the lower left corner to the upper right
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glUseProgram(simpleRenderProgramID);
		glViewport(0, 0, windowWidth / 2, windowHeight);


		// Compute the MVP matrix from keyboard and mouse input

		// Compute the MVP matrix from keyboard and mouse input
		// computeMatricesFromInputs();
		glm::vec3 modelOrigin = glm::vec3(0, 0, 0);
		computeMatricesFromRotationOverTime(modelOrigin);
		glm::mat4 ProjectionMatrix = getProjectionMatrix();
		glm::mat4 ViewMatrix = getViewMatrix();
		glm::mat4 ModelMatrix = glm::mat4(1.0);
		glm::mat4 ModelViewMatrix = ViewMatrix * ModelMatrix;
		glm::mat3 ModelView3x3Matrix = glm::mat3(ModelViewMatrix);
		glm::mat4 MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;

		// Send our transformation to the currently bound shader, 
		// in the "MVP" uniform
		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
		glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);
		glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &ViewMatrix[0][0]);
		glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &ViewMatrix[0][0]);
		glUniformMatrix3fv(ModelView3x3MatrixID, 1, GL_FALSE, &ModelView3x3Matrix[0][0]);


		glm::vec3 lightPos = glm::vec3(0, 0, 4);
		glUniform3f(LightID, lightPos.x, lightPos.y, lightPos.z);

		// Bind our diffuse texture in Texture Unit 0
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, DiffuseTexture);
		// Set our "DiffuseTextureSampler" sampler to use Texture Unit 0
		glUniform1i(DiffuseTextureID, 0);


		// 1rst attribute buffer : vertices
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, obj_vertexbuffer);
		glVertexAttribPointer(
			0,                  // attribute
			3,                  // size
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
		);

		// 2nd attribute buffer : UVs
		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, obj_uvbuffer);
		glVertexAttribPointer(
			1,                                // attribute
			2,                                // size
			GL_FLOAT,                         // type
			GL_FALSE,                         // normalized?
			0,                                // stride
			(void*)0                          // array buffer offset
		);

		// 3rd attribute buffer : normals
		glEnableVertexAttribArray(2);
		glBindBuffer(GL_ARRAY_BUFFER, obj_normalbuffer);
		glVertexAttribPointer(
			2,                                // attribute
			3,                                // size
			GL_FLOAT,                         // type
			GL_FALSE,                         // normalized?
			0,                                // stride
			(void*)0                          // array buffer offset
		);

		glEnableVertexAttribArray(3);
		glBindBuffer(GL_ARRAY_BUFFER, obj_tangentbuffer);
		glVertexAttribPointer(
			3,                                // attribute
			3,                                // size
			GL_FLOAT,                         // type
			GL_FALSE,                         // normalized?
			0,                                // stride
			(void*)0                          // array buffer offset
		);

		glEnableVertexAttribArray(4);
		glBindBuffer(GL_ARRAY_BUFFER, obj_bitangentbuffer);
		glVertexAttribPointer(
			4,                                // attribute
			3,                                // size
			GL_FLOAT,                         // type
			GL_FALSE,                         // normalized?
			0,                                // stride
			(void*)0                          // array buffer offset
		);

		// Index buffer
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, obj_elementbuffer);
		// Draw the triangles !
		glDrawElements(
			GL_TRIANGLES,      // mode
			indexed_indices.size(),    // count
			GL_UNSIGNED_SHORT, // type
			(void*)0           // element array buffer offset
		);


		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glViewport(0, 0, windowWidth / 2, windowHeight);
		glDrawElements(
			GL_TRIANGLES,      // mode
			indexed_indices.size(),    // count
			GL_UNSIGNED_SHORT, // type
			(void*)0           // element array buffer offset
		);

		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);
		glDisableVertexAttribArray(2);
		glDisableVertexAttribArray(3);
		glDisableVertexAttribArray(4);

		// Rebind the frame buffer to the screen
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(windowWidth / 2, 0, windowWidth / 2, windowHeight);
		// Render on the whole framebuffer, complete from the lower left corner to the upper right
		//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Use our shader
		glUseProgram(postProcessProgramId);

		// Bind our texture in Texture Unit 0
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, RenderedTexture);
		glUniform1i(RenderedTextureID, 0);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, BrushTexture);
		// Set our "BrushTextureSampler" sampler to use Texture Unit 1
		glUniform1i(BrushTextureID, 1);

		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, BrushTangentTexture);
		glUniform1i(BrushTangentTextureID, 2);

		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, BrushBitangentTexture);
		glUniform1i(BrushBitangentTextureID, 3);

		glUniform3fv(BackgroundColorID, 1, &BACKGROUND_COLOR[0]);

		// glUniformMatrix3fv(BrushScaleMatrixID, 1, GL_FALSE, &brushScaleMat[0][0]);
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, quad_vertexbuffer);
		glVertexAttribPointer(
			0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
			3,                  // size
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
		);

		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, quad_uvbuffer);
		glVertexAttribPointer(
			1,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
			2,                  // size
			GL_FLOAT,				// type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
		);

		glEnableVertexAttribArray(2);
		glBindBuffer(GL_ARRAY_BUFFER, quad_brushwidths_buffer);
		glVertexAttribPointer(
			2,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
			1,                  // size
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
		);

		glEnableVertexAttribArray(3);
		glBindBuffer(GL_ARRAY_BUFFER, quad_brushlengths_buffer);
		glVertexAttribPointer(
			3,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
			1,                  // size
			GL_FLOAT,				// type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
		);

		glEnableVertexAttribArray(4);
		glBindBuffer(GL_ARRAY_BUFFER, quad_brushrotation_buffer);
		glVertexAttribPointer(
			4,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
			1,                  // size
			GL_FLOAT,				// type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
		);

		glEnableVertexAttribArray(5);
		glBindBuffer(GL_ARRAY_BUFFER, quad_colorvariance_buffer);
		glVertexAttribPointer(
			5,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
			3,                  // size
			GL_FLOAT,				// type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
		);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, quad_elementbuffer);

		// Draw the triangles !
		glDrawElements(
			GL_TRIANGLES,      // mode
			g_quad_indices.size(),    // count
			GL_UNSIGNED_SHORT, // type
			(void*)0           // element array buffer offset
		);

		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);
		glDisableVertexAttribArray(2);
		glDisableVertexAttribArray(3);
		glDisableVertexAttribArray(4);
		glDisableVertexAttribArray(5);

		// Swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents();

	} // Check if the ESC key was pressed or the window was closed
	while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
		glfwWindowShouldClose(window) == 0);


	// Cleanup VBO and shader
	glDeleteBuffers(1, &obj_vertexbuffer);
	glDeleteBuffers(1, &obj_uvbuffer);
	glDeleteBuffers(1, &obj_normalbuffer);
	glDeleteBuffers(1, &obj_elementbuffer);
	glDeleteProgram(simpleRenderProgramID);
	glDeleteTextures(1, &DiffuseTexture);
	glDeleteVertexArrays(1, &VertexArrayID);

	// Close OpenGL window and terminate GLFW
	glfwTerminate();

	return 0;
}