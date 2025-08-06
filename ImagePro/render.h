#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <windows.h>
#include <commdlg.h>

#include "common/string_utils.hpp"

#include "int.h"

// ImGui headers
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#define GLEW_STATIC
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace glm;

#include "common/shader.hpp"
#include "common/texture.hpp"

#include "agedetection.h"
#include "license.h"

#include "editcommand.h"

namespace imagepro
{
	class CRender
	{
	public:
		CRender();

		void Shutdown();

		void Run();

	private:
		void OpenURL(string endpoint);

		GLuint LoadTexture(const char* filePathName);

		void OpenFileDialog();

		std::string OpenSaveFileDialog();

		void ResetSliderValues();

		static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);

		void mouse_button_callback_impl(GLFWwindow* window, int button, int action, int mods);

		static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos);

		void cursor_position_callback_impl(GLFWwindow* window, double xpos, double ypos);

		static void scroll_callback(GLFWwindow* window, double xOffset, double yOffset);

		void scroll_callback_impl(GLFWwindow* window, double xOffset, double yOffset);

		std::vector<unsigned char> ReadOpenGLFramebufferToBGRA(int width, int height);

		void CopyPixelsToClipboard(int width, int height, const std::vector<unsigned char>& bgraPixels);

		bool SavePixelsToFile(int width, int height, const std::vector<unsigned char>& bgraPixels, const std::string& filename);

		int Init();

		void SetIcon();

		void ProcessInput();

		void UpdateView();

		void DoUI();

		void MainLoop();

		//
		// class varables
		//

		bool programRunning = true;

		bool showOptions = true;
		bool showLicenseAlertChecking = false;
		bool adultAlertShowFlag = false;
		bool showAboutDialog = false;
		bool showLicenseDialog = false;
		bool showTermsOfUse = false;

		bool showColorPicker = false;
		ImVec4 colorPicker = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);

		float offsetSlider = 0.0025f;
		bool rSliderChecked = false;
		float rSlider = 1;
		bool gSliderChecked = false;
		float gSlider = 1;
		bool bSliderChecked = false;
		float bSlider = 1;
		float zoomSlider = 100;
		float zoomScroolScaler = 5.0f;
		float xOffsetSlider = 0;
		float yOffsetSlider = 0;
		float deltaScaler = 0.003f;
		float zoomSliderShader = 0;
		float xOffsetSliderShader = 0;
		float yOffsetSliderShader = 0;

		//
		// input flags
		//
		bool onMiddleButton = false;
		float lastX = 0.0;
		float lastY = 0.0;
		float deltaX = 0.0;
		float deltaY = 0.0;

		//
		// polygon stuff
		//
		float xScaler = 1;
		float yScaler = 1;
		int width;
		int height;

		GLuint vertexbuffer;
		GLuint uvbuffer;
		GLuint VertexArrayID;

		GLfloat vSz = 0.9f;

		float modelRotationInDegrees = 0;

		// if vSz = 0.5
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
		//
		// -----------------------------
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


		// Vertex buffer data for two triangles (forming a square in 2D)
		GLfloat g_vertex_buffer_data[18] = {
			// First Triangle (Bottom triangle of the square)
			-vSz, -vSz, 0, // Vertex 1
			 vSz, -vSz, 0, // Vertex 2
			 vSz,  vSz, 0, // Vertex 3

			 // Second Triangle (Top-right triangle of the square)
			 -vSz, -vSz, 0, // Vertex 4
			  vSz,  vSz, 0, // Vertex 5
			 -vSz,  vSz, 0 // Vertex 6
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

		//
		// gl stuff
		//
		GLFWwindow* mp_Window;
		GLuint mi_Texture;
		uintptr_t abxPtr;

		int mi_WindowWidth = 1280;
		int mi_WindowHeight = 720;

		GLuint alienProgramID;

		unique_ptr<ImageProEditCommand> imageProEditCommand;

		bool copyCurrentImageToClipboardFlag = false;
		bool saveImageToFileFlag = false;
		string saveImageToFileName = "";
	};
}