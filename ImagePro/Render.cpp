// Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <windows.h>
#include <commdlg.h>

#include "common/string_utils.hpp"

// Then include ImGui headers
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

// Include GLEW
#include <GL/glew.h>

// Include GLFW
#include <GLFW/glfw3.h>

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace glm;

#include "common/shader.hpp"
#include "common/texture.hpp"

namespace imagepro
{
	class CRender
	{
	public:
		GLuint LoadTexture(const char* filePathName)
		{
			GLuint texture = LoadTextureFromFile(filePathName, width, height);

			xScaler = width / (float)height;
			yScaler = height / (float)width;
			return texture;
		}

		void OpenFileDialog() 
		{
			OPENFILENAME ofn;
			TCHAR szFile[260] = { 0 };

			ZeroMemory(&ofn, sizeof(ofn));
			ofn.lStructSize = sizeof(ofn);
			ofn.hwndOwner = NULL;
			ofn.lpstrFile = szFile;
			ofn.nMaxFile = sizeof(szFile);
			ofn.lpstrFilter = L"PNG Images (*.png)\0*.png\0JPEG Images (*.jpg;*.jpeg)\0*.jpg;*.jpeg\0GIF Images (*.gif)\0*.gif;\0BMP Images (*.bmp)\0*.bmp;\0";
			ofn.nFilterIndex = 1;
			ofn.lpstrFileTitle = NULL;
			ofn.nMaxFileTitle = 0;
			ofn.lpstrInitialDir = NULL;
			ofn.lpstrTitle = L"Select a file";
			ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

			// Show the dialog box
			if (GetOpenFileName(&ofn) == TRUE) 
			{
				std::cout << "Selected file: " << LPWSTRToChar(ofn.lpstrFile) << std::endl;
				mi_Texture = LoadTexture(LPWSTRToChar(ofn.lpstrFile));
			}
		}

		void resetSliderValues()
		{
			rSliderChecked = true;
			gSliderChecked = true;
			bSliderChecked = true;

			rSlider = 1;
			gSlider = 1;
			bSlider = 1;

			zoomSlider = 0;
			xOffsetSlider = 0;
			yOffsetSlider = 0;
		}

		static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
		{
			CRender* obj = static_cast<CRender*>(glfwGetWindowUserPointer(window));

			if (obj)
			{
				obj->mouse_button_callback_impl(window, button, action, mods);
			}
		}

		void mouse_button_callback_impl(GLFWwindow* window, int button, int action, int mods) 
		{
			if (action == GLFW_PRESS) 
			{
				if (button == GLFW_MOUSE_BUTTON_LEFT) 
				{
				}
				else if (button == GLFW_MOUSE_BUTTON_RIGHT) 
				{
				}
				else if (button == GLFW_MOUSE_BUTTON_MIDDLE) 
				{
					glfwSetInputMode(window, GLFW_CURSOR, GLFW_CROSSHAIR_CURSOR);
					onMiddleButton = true;
				}
			}
			if (action == GLFW_RELEASE)
			{
				if (button == GLFW_MOUSE_BUTTON_LEFT) 
				{
				}
				else if (button == GLFW_MOUSE_BUTTON_RIGHT) 
				{
				}
				else if (button == GLFW_MOUSE_BUTTON_MIDDLE) 
				{
					glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
					onMiddleButton = false;
				}
			}
		}

		static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
		{
			CRender* obj = static_cast<CRender*>(glfwGetWindowUserPointer(window));

			if (obj)
			{
				obj->cursor_position_callback_impl(window, xpos, ypos);
			}
		}


		void cursor_position_callback_impl(GLFWwindow* window, double xpos, double ypos) 
		{
			deltaX = xpos - lastX;
			deltaY = ypos - lastY;

			// Update last position
			lastX = xpos;
			lastY = ypos;

			if (onMiddleButton)
			{
				xOffsetSlider -= deltaX * deltaScaler;
				yOffsetSlider -= deltaY * deltaScaler;
			}
		}

		static void scroll_callback(GLFWwindow* window, double xOffset, double yOffset)
		{
			CRender* obj = static_cast<CRender*>(glfwGetWindowUserPointer(window));

			if (obj)
			{
				obj->scroll_callback_impl(window, xOffset, yOffset);
			}
		}

		void scroll_callback_impl(GLFWwindow* window, double xOffset, double yOffset) 
		{
			zoomSlider += yOffset * zoomScroolScaler;

			if (zoomSlider < 0) {
				zoomSlider = 0;
			}
			if (zoomSlider > 20) {
				zoomSlider = 20;
			}
		}

		int Init()
		{
			// Initialize GLFW
			if (!glfwInit())
			{
				fprintf(stderr, "Failed to initialize GLFW\n");
				return -1;
			}

			glfwWindowHint(GLFW_SAMPLES, 4);
			glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
			glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
			glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make macOS happy; should not be needed
			glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

			//
			// Open a window and create its OpenGL context
			//
			mp_Window = glfwCreateWindow(mi_WindowWidth, mi_WindowHeight, "ImagePro", NULL, NULL);
			if (mp_Window == NULL) 
			{
				fprintf(stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible\n");
				glfwTerminate();
				return -1;
			}

			glfwSetWindowUserPointer(mp_Window, this);

			glfwSetMouseButtonCallback(mp_Window, mouse_button_callback);
			glfwSetCursorPosCallback(mp_Window, cursor_position_callback);
			glfwSetScrollCallback(mp_Window, scroll_callback);

			glfwMakeContextCurrent(mp_Window);

			// Initialize GLEW
			glewExperimental = true; // Needed for core profile
			if (glewInit() != GLEW_OK) 
			{
				fprintf(stderr, "Failed to initialize GLEW\n");
				return -1;
			}

			// Ensure we can capture the escape key being pressed below
			glfwSetInputMode(mp_Window, GLFW_STICKY_KEYS, GL_TRUE);

			// Set the mouse at the center of the screen
			glfwPollEvents();
			glfwSetCursorPos(mp_Window, mi_WindowWidth / 2, mi_WindowHeight / 2);

			// Dark blue background
			glClearColor(0.0f, 0.0f, 0.2f, 0.0f);

			// Enable depth test
			glEnable(GL_DEPTH_TEST);

			// Accept fragment if it is closer to the camera than the former one
			glDepthFunc(GL_LESS);

			glGenVertexArrays(1, &VertexArrayID);
			glBindVertexArray(VertexArrayID);


			//
			// IMG GUI --------------------------------------------------------
			//
			IMGUI_CHECKVERSION();
			ImGui::CreateContext();
			ImGuiIO& io = ImGui::GetIO();
			(void)io;
			ImGui::StyleColorsDark();
			ImGui_ImplGlfw_InitForOpenGL(mp_Window, true);
			ImGui_ImplOpenGL3_Init("#version 330");

			// ----------------------------------------------------------------

			//
			// alien map shader
			//
			alienProgramID = LoadShaders("AlienMap.vertexshader", "AlienMap.fragmentshader");

			// Projection matrix : 45º Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
			float aspect = 4.0f / 3.0f;
			//float aspect = mi_WindowWidth / (float)mi_WindowHeight;
			glm::mat4 Projection = glm::perspective(glm::radians(45.0f), aspect, 0.1f, 100.0f);
			
			//float aspect = mi_WindowWidth / (float)mi_WindowHeight;

			// Define near and far planes
			float nearPlane = 0.1f;
			float farPlane = 100.0f;

			// Define the boundaries of the orthographic projection
			float left = -1.0f;
			float right = 1.0f;
			float bottom = -1.0f;
			float top = 1.0f;

			aspect = 0;

			// Adjust the projection based on the aspect ratio
			if (aspect > 1.0f) 
			{
				// Wider than tall, adjust left and right
				left = -aspect;
				right = aspect;
			}
			else 
			{
				// Taller than wide, adjust top and bottom
				bottom = -1.0f / aspect;
				top = 1.0f / aspect;
			}

			// Create the orthographic projection matrix
			glm::mat4 Projection2 = glm::ortho(left, right, bottom, top, nearPlane, farPlane);

			// Camera matrix
			glm::mat4 View = glm::lookAt(
				glm::vec3(0, 0, 1.5f), // Camera is at (4,3,3), in World Space
				glm::vec3(0, 0, 0), // and looks at the origin
				glm::vec3(0, 1, 0)  // Head is up (set to 0,-1,0 to look upside-down)
			);
			// Model matrix : an identity matrix (model will be at the origin)
			glm::mat4 Model = glm::mat4(1.0f);
			// Our ModelViewProjection : multiplication of our 3 matrices
			glm::mat4 MVP = Projection * View * Model; // Remember, matrix multiplication is the other way around

			// Load the texture using any two methods
			mi_Texture = LoadTexture("uvtemplate.png");

			glGenBuffers(1, &vertexbuffer);
			glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
			glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);

			glGenBuffers(1, &uvbuffer);
			glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
			glBufferData(GL_ARRAY_BUFFER, sizeof(g_uv_buffer_data), g_uv_buffer_data, GL_STATIC_DRAW);
		}

		void MainLoop()
		{
			do
			{
				//
				// process input
				//
				{
					if (glfwGetKey(mp_Window, GLFW_KEY_R) == GLFW_PRESS)
					{
						rSlider += offsetSlider;
					}
					if (glfwGetKey(mp_Window, GLFW_KEY_E) == GLFW_PRESS)
					{
						rSlider -= offsetSlider;
					}
					if (glfwGetKey(mp_Window, GLFW_KEY_G) == GLFW_PRESS)
					{
						gSlider += offsetSlider;
					}
					if (glfwGetKey(mp_Window, GLFW_KEY_F) == GLFW_PRESS)
					{
						gSlider -= offsetSlider;
					}
					if (glfwGetKey(mp_Window, GLFW_KEY_B) == GLFW_PRESS)
					{
						bSlider += offsetSlider;
					}
					if (glfwGetKey(mp_Window, GLFW_KEY_V) == GLFW_PRESS)
					{
						bSlider -= offsetSlider;
					}
				}


				// Clear the screen
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

				//
				// ImGui
				//

				ImGui_ImplOpenGL3_NewFrame();
				ImGui_ImplGlfw_NewFrame();
				ImGui::NewFrame();


				// Bind our texture in Texture Unit 0
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, mi_Texture);

				//
				// Use our shader
				//
				glUseProgram(alienProgramID);

				//glm::vec3 colorTint(1.0f, 0.5f, 0.5f);  // Red tint
				//glUniform3fv(glGetUniformLocation(alienProgramID, "colorTint"), 1, &colorTint[0]);

				GLint freqLocation = glGetUniformLocation(alienProgramID, "freq");
				GLint phaseShiftLocation = glGetUniformLocation(alienProgramID, "phaseshift");
				GLint keepLocation = glGetUniformLocation(alienProgramID, "keep");

				// Example of setting uniform values (set based on your needs)
				glUniform3f(freqLocation, rSlider, gSlider, bSlider);
				glUniform3f(phaseShiftLocation, 0.0f, 0.0f, 0.0f);
				glUniform3f(keepLocation, rSliderChecked ? 1 : 0, gSliderChecked ? 1 : 0, bSliderChecked ? 1 : 0);

				//
				// zoom offset
				//
				GLuint zoomLoc = glGetUniformLocation(alienProgramID, "zoom");
				GLuint offsetLoc = glGetUniformLocation(alienProgramID, "offset");

				//
				// polygon x y scale
				//
				GLint xScalerLocation = glGetUniformLocation(alienProgramID, "xScaler");
				GLint yScalerLocation = glGetUniformLocation(alienProgramID, "yScaler");
				glUniform1f(xScalerLocation, xScaler);
				glUniform1f(yScalerLocation, yScaler);

				// Set the zoom and offset values
				glm::vec2 offset(xOffsetSlider, yOffsetSlider);

				// Send uniforms to the shader
				glUniform1f(zoomLoc, 1 - zoomSlider / 20.0f);
				glUniform2fv(offsetLoc, 1, &offset[0]);


				glUniform1i(glGetUniformLocation(alienProgramID, "inputTexture"), 0);

				// Set our "myTextureSampler" sampler to use Texture Unit 0
				//glUniform1i(TextureID, 0);

				// 1rst attribute buffer : vertices
				glEnableVertexAttribArray(0);
				glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
				glVertexAttribPointer(
					0,                  // attribute. No particular reason for 0, but must match the layout in the shader.
					3,                  // size
					GL_FLOAT,           // type
					GL_FALSE,           // normalized?
					0,                  // stride
					(void*)0            // array buffer offset
				);

				// 2nd attribute buffer : UVs
				glEnableVertexAttribArray(1);
				glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
				glVertexAttribPointer(
					1,                                // attribute. No particular reason for 1, but must match the layout in the shader.
					2,                                // size : U+V => 2
					GL_FLOAT,                         // type
					GL_FALSE,                         // normalized?
					0,                                // stride
					(void*)0                          // array buffer offset
				);

				// Draw the triangle !
				glDrawArrays(GL_TRIANGLES, 0, 12 * 3); // 12*3 indices starting at 0 -> 12 triangles


				glDisableVertexAttribArray(0);
				glDisableVertexAttribArray(1);

				// ------------------------------------------------------------------------
				//
				// ImGui creation
				//

				ImGui::SetNextWindowSize(ImVec2(290, 360));
				ImGui::Begin("Options");
				ImGui::Text("Color mix");
				ImGui::Checkbox("Use Red", &rSliderChecked);
				ImGui::SliderFloat("Red", &rSlider, 0.0f, 20.0f);
				ImGui::Checkbox("Use Green", &gSliderChecked);
				ImGui::SliderFloat("Green", &gSlider, 0.0f, 20.0f);
				ImGui::Checkbox("Use Blue", &bSliderChecked);
				ImGui::SliderFloat("Blue", &bSlider, 0.0f, 20.0f);
				ImGui::Text("Zoom");
				ImGui::SliderFloat("Zoom", &zoomSlider, 0.0f, 20.0f);
				ImGui::Text("Offset");
				ImGui::SliderFloat("X Offset", &xOffsetSlider, -1.0f, 1.0f);
				ImGui::SliderFloat("Y Offset", &yOffsetSlider, -1.0f, 1.0f);
				ImGui::Spacing();
				ImGui::Spacing();
				if (ImGui::Button("Reset all values"))
				{
					resetSliderValues();
				}
				ImGui::Spacing();
				ImGui::Spacing();
				ImGui::Spacing();
				if (ImGui::Button("Open an Image..."))
				{
					OpenFileDialog();
				}
				ImGui::End();

				ImGui::Render();
				ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

				// ------------------------------------------------------------------------

				// Swap buffers
				glfwSwapBuffers(mp_Window);
				glfwPollEvents();

			}
			while (glfwGetKey(mp_Window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
				glfwWindowShouldClose(mp_Window) == 0);
		}

		void Shutdown()
		{
			ImGui_ImplOpenGL3_Shutdown();
			ImGui_ImplGlfw_Shutdown();
			ImGui::DestroyContext();

			// Cleanup VBO and shader
			glDeleteBuffers(1, &vertexbuffer);
			glDeleteBuffers(1, &uvbuffer);
			//glDeleteProgram(programID);
			glDeleteProgram(alienProgramID);
			glDeleteTextures(1, &mi_Texture);
			glDeleteVertexArrays(1, &VertexArrayID);

			// Close OpenGL window and terminate GLFW
			glfwTerminate();
		}

		void Run()
		{
			Init();

			MainLoop();

			Shutdown();
		}

		//
		// variables
		//

		// gui variables
		float offsetSlider = 0.0025f;
		bool rSliderChecked = true;
		float rSlider = 1;
		bool gSliderChecked = true;
		float gSlider = 1;
		bool bSliderChecked = true;
		float bSlider = 1;

		float zoomSlider = 0;
		float zoomScroolScaler = 0.25f;

		float xOffsetSlider = 0;
		float yOffsetSlider = 0;

		float deltaScaler = 0.0008f;

		
		// input flags
		bool onMiddleButton = false;
		float lastX = 0.0;
		float lastY = 0.0;
		float deltaX = 0.0;
		float deltaY = 0.0;

		// polygon scale
		float xScaler = 1;
		float yScaler = 1;
		int width;
		int height;


		//
		// 
		// BOTTOM
		//                   vertex 3
		//                  (0.5, 0.5) 
		//                      .
		//                     /|
		//                   /  |
		//                 /    |
		//               /      |
		//             /        |
		//           /          |
		//         /            |
		//        .-------------.  
		//  (-0.5, -0.5)    (0.5, -0.5)
		//    vertex 1        vertex 2
		// 
		// -----------------------------
		// 
		// TOP
		//    vertex 5
		//   (0.5, 0.5) 
		//        .
		//        |\
		//        |  \
		//        |    \
		//        |      \
		//        |        \
		//        |          \
		//        |            \
		//        .-------------.  
		//  (-0.5, -0.5)    (0.5, -0.5)
		//   vertex 4         vertex 6

		GLuint vertexbuffer;
		GLuint uvbuffer;
		GLuint VertexArrayID;

		// Vertex buffer data for two triangles (forming a square in 2D)
		GLfloat g_vertex_buffer_data[18] = {
			// First Triangle (Bottom triangle of the square)
			-0.5f, -0.5f, 0.0f, // Vertex 1
			 0.5f, -0.5f, 0.0f, // Vertex 2
			 0.5f,  0.5f, 0.0f, // Vertex 3

			 // Second Triangle (Top-right triangle of the square)
			 -0.5f, -0.5f, 0.0f, // Vertex 4
			  0.5f,  0.5f, 0.0f, // Vertex 5
			 -0.5f,  0.5f, 0.0f  // Vertex 6
		};

		// UV buffer data for the texture coordinates
		GLfloat g_uv_buffer_data[12] = {
			0.0f, 1.0f, // UV for Vertex 1 (flipped)
			1.0f, 1.0f, // UV for Vertex 2 (flipped)
			1.0f, 0.0f, // UV for Vertex 3 (flipped)

			// Second Triangle
			0.0f, 1.0f, // UV for Vertex 4 (flipped)
			1.0f, 0.0f, // UV for Vertex 5 (flipped)
			0.0f, 0.0f  // UV for Vertex 6 (flipped)
		};

		// gl stuff
		GLFWwindow* mp_Window;
		GLuint mi_Texture;

		int mi_WindowWidth = 1280;
		int mi_WindowHeight = 720;

		GLuint alienProgramID;
	};
}

int main(void)
{
	using namespace imagepro;
	using namespace std;

	CRender* render = new CRender();
	render->Run();
	delete render;

	return 0;
}

