// Include standard headers
#include "render.h"

namespace imagepro
{
	CRender::CRender()
	{
		this->imageProEditCommand = std::make_unique<ImageProEditCommand>(this);
	}

	void CRender::Shutdown()
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

	void CRender::Run()
	{
		std::srand(static_cast<unsigned>(std::time(nullptr)));
		Int* realPtr = new Int(0);
		abxPtr = reinterpret_cast<uintptr_t>(realPtr) ^ 0xA5A5A5A5;

		Init();

		MainLoop();

		Shutdown();

		Int* pi = reinterpret_cast<Int*>(abxPtr ^ 0xA5A5A5A5);
		delete pi;
	}

	void CRender::OpenURL(string endpoint)
	{
#ifdef _WIN32
		ShellExecuteA(nullptr, "open", endpoint.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
#elif __APPLE__
		system(endpoint);
#else // Linux
		system(endpoint);
#endif
	}

	GLuint CRender::LoadTexture(const char* filePathName)
	{
		GLuint texture = LoadTextureFromFile(filePathName, width, height);

		xScaler = width / (float)height;
		yScaler = height / (float)width;
		return texture;
	}

	void CRender::OpenFileDialog()
	{
		OPENFILENAME ofn;
		TCHAR szFile[260] = { 0 };

		ZeroMemory(&ofn, sizeof(ofn));
		ofn.lStructSize = sizeof(ofn);
		ofn.hwndOwner = NULL;
		ofn.lpstrFile = szFile;
		ofn.nMaxFile = sizeof(szFile);
		ofn.lpstrFilter = L"JPEG Images (*.jpg;*.jpeg)\0*.jpg;*.jpeg\0PNG Images (*.png)\0*.png\0GIF Images (*.gif)\0*.gif;\0BMP Images (*.bmp)\0*.bmp;\0";
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

	std::string CRender::OpenSaveFileDialog()
	{
		OPENFILENAME ofn;
		WCHAR szFile[MAX_PATH] = L"";

		ZeroMemory(&ofn, sizeof(ofn));
		ofn.lStructSize = sizeof(ofn);
		ofn.hwndOwner = NULL; // Set to your app window if needed
		ofn.lpstrFile = szFile;
		ofn.nMaxFile = MAX_PATH;
		ofn.lpstrFilter =
			L"PNG Images (*.png)\0*.png\0";
		ofn.nFilterIndex = 1;
		ofn.lpstrTitle = L"Save Screenshot As";
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;

		if (GetSaveFileName(&ofn) == TRUE)
		{
			string prefix = LPWSTRToChar(ofn.lpstrFile);
			return prefix + ".png";
		}

		return "";
	}

	void CRender::ResetSliderValues()
	{
		rSliderChecked = false;
		gSliderChecked = false;
		bSliderChecked = false;

		rSlider = 1;
		gSlider = 1;
		bSlider = 1;

		zoomSlider = 100;
		xOffsetSlider = 0;
		yOffsetSlider = 0;

		zoomSliderShader = 0;
		xOffsetSliderShader = 0;
		yOffsetSliderShader = 0;
	}

	void CRender::mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
	{
		CRender* obj = static_cast<CRender*>(glfwGetWindowUserPointer(window));

		if (obj)
		{
			obj->mouse_button_callback_impl(window, button, action, mods);
		}
	}

	void CRender::mouse_button_callback_impl(GLFWwindow* window, int button, int action, int mods)
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

	void CRender::cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
	{
		CRender* obj = static_cast<CRender*>(glfwGetWindowUserPointer(window));

		if (obj)
		{
			obj->cursor_position_callback_impl(window, xpos, ypos);
		}
	}

	void CRender::cursor_position_callback_impl(GLFWwindow* window, double xpos, double ypos)
	{
		float xposF = static_cast<float>(xpos);
		float yposF = static_cast<float>(ypos);

		deltaX = xposF - lastX;
		deltaY = yposF - lastY;

		// update last position
		lastX = xposF;
		lastY = yposF;

		if (onMiddleButton)
		{
			xOffsetSlider += deltaX * deltaScaler;
			yOffsetSlider -= deltaY * deltaScaler;
		}
	}

	void CRender::scroll_callback(GLFWwindow* window, double xOffset, double yOffset)
	{
		CRender* obj = static_cast<CRender*>(glfwGetWindowUserPointer(window));

		if (obj)
		{
			obj->scroll_callback_impl(window, xOffset, yOffset);
		}
	}

	void CRender::scroll_callback_impl(GLFWwindow* window, double xOffset, double yOffset)
	{
		zoomSlider += static_cast<float>(yOffset * zoomScroolScaler);

		if (zoomSlider < 1) {
			zoomSlider = 1;
		}
		if (zoomSlider > 500) {
			zoomSlider = 500;
		}
	}

	std::vector<unsigned char> CRender::ReadOpenGLFramebufferToBGRA(int width, int height)
	{
		std::vector<unsigned char> rgbaPixels(width * height * 4);
		glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, rgbaPixels.data());

		std::vector<unsigned char> bgraPixels(width * height * 4);
		for (int i = 0; i < width * height; ++i)
		{
			bgraPixels[i * 4 + 0] = rgbaPixels[i * 4 + 2]; // Blue
			bgraPixels[i * 4 + 1] = rgbaPixels[i * 4 + 1]; // Green
			bgraPixels[i * 4 + 2] = rgbaPixels[i * 4 + 0]; // Red
			bgraPixels[i * 4 + 3] = 0;                     // Alpha ignored
		}
		return bgraPixels;
	}

	void CRender::CopyPixelsToClipboard(int width, int height, const std::vector<unsigned char>& bgraPixels)
	{
		int rowStride = width * 4;
		int imageSize = rowStride * height;
		int dibHeaderSize = sizeof(BITMAPINFOHEADER);
		HGLOBAL hGlobal = GlobalAlloc(GMEM_MOVEABLE, dibHeaderSize + imageSize);
		if (!hGlobal)
			return;

		void* ptr = GlobalLock(hGlobal);
		if (!ptr)
		{
			GlobalFree(hGlobal);
			return;
		}

		BITMAPINFOHEADER* header = (BITMAPINFOHEADER*)ptr;
		header->biSize = sizeof(BITMAPINFOHEADER);
		header->biWidth = width;
		header->biHeight = height;
		header->biPlanes = 1;
		header->biBitCount = 32;
		header->biCompression = BI_RGB;
		header->biSizeImage = imageSize;
		header->biXPelsPerMeter = 2835;
		header->biYPelsPerMeter = 2835;
		header->biClrUsed = 0;
		header->biClrImportant = 0;

		unsigned char* dstPixels = (unsigned char*)ptr + dibHeaderSize;
		memcpy(dstPixels, bgraPixels.data(), imageSize);

		GlobalUnlock(hGlobal);

		if (OpenClipboard(NULL))
		{
			EmptyClipboard();
			SetClipboardData(CF_DIB, hGlobal);
			CloseClipboard();
		}
		else
		{
			GlobalFree(hGlobal);
		}
	}

	bool CRender::SavePixelsToFile(int width, int height, const std::vector<unsigned char>& bgraPixels, const std::string& filename)
	{
		// ensure dimensions are valid
		if (width <= 0 || height <= 0 || bgraPixels.size() != static_cast<size_t>(width * height * 4))
		{
			return false;
		}

		// File header (14 bytes)
		BITMAPFILEHEADER fileHeader = {};
		fileHeader.bfType = 0x4D42; // 'BM'
		fileHeader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

		// Info header (40 bytes)
		BITMAPINFOHEADER infoHeader = {};
		infoHeader.biSize = sizeof(BITMAPINFOHEADER);
		infoHeader.biWidth = width;
		infoHeader.biHeight = height; // positive = bottom-up
		infoHeader.biPlanes = 1;
		infoHeader.biBitCount = 32;
		infoHeader.biCompression = BI_RGB;
		infoHeader.biSizeImage = width * height * 4;

		fileHeader.bfSize = fileHeader.bfOffBits + infoHeader.biSizeImage;

		std::ofstream file(filename, std::ios::binary);
		if (!file)
			return false;

		// write headers
		file.write(reinterpret_cast<const char*>(&fileHeader), sizeof(fileHeader));
		file.write(reinterpret_cast<const char*>(&infoHeader), sizeof(infoHeader));

		for (int y = 0; y < height; ++y)
		{
			const unsigned char* row = &bgraPixels[y * width * 4];
			file.write(reinterpret_cast<const char*>(row), width * 4);
		}

		return file.good();
	}

	int CRender::Init()
	{
		showLicenseAlertChecking = true;
		License::getInstance().checkLicense(abxPtr, true);

		// initialize GLFW
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
		// open a window and create its OpenGL context
		//
		mp_Window = glfwCreateWindow(mi_WindowWidth, mi_WindowHeight, "ImagePro", NULL, NULL);
		if (mp_Window == NULL)
		{
			fprintf(stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible\n");
			glfwTerminate();
			return -1;
		}

		SetIcon();

		glfwSetWindowUserPointer(mp_Window, this);

		glfwSetMouseButtonCallback(mp_Window, mouse_button_callback);
		glfwSetCursorPosCallback(mp_Window, cursor_position_callback);
		glfwSetScrollCallback(mp_Window, scroll_callback);

		glfwMakeContextCurrent(mp_Window);

		// initialize GLEW
		glewExperimental = true; // Needed for core profile
		if (glewInit() != GLEW_OK)
		{
			fprintf(stderr, "Failed to initialize GLEW\n");
			return -1;
		}

		// ensure we can capture the escape key being pressed below
		glfwSetInputMode(mp_Window, GLFW_STICKY_KEYS, GL_TRUE);

		// set the mouse at the center of the screen
		glfwPollEvents();
		glfwSetCursorPos(mp_Window, mi_WindowWidth / 2, mi_WindowHeight / 2);

		// background
		glClearColor(colorPicker.x, colorPicker.y, colorPicker.z, 0.0f);

		// enable depth test
		glEnable(GL_DEPTH_TEST);

		// accept fragment if it is closer to the camera than the former one
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

		// Load the texture using any two methods
		mi_Texture = LoadTexture("uvtemplate.png");

		glGenBuffers(1, &vertexbuffer);
		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);

		glGenBuffers(1, &uvbuffer);
		glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(g_uv_buffer_data), g_uv_buffer_data, GL_STATIC_DRAW);

		return 0;
	}

    void CRender::SetIcon()
    {
       if (!mp_Window) // Ensure mp_Window is initialized
       {
           std::cerr << "Error: mp_Window is not initialized!" << std::endl;
           return;
       }

       HWND hwnd = glfwGetWin32Window(mp_Window);

	   HICON hNewIcon = (HICON)LoadImage(
		   NULL,
		   L"imagepro.ico",
		   IMAGE_ICON,
		   0, 0,
		   LR_LOADFROMFILE | LR_DEFAULTSIZE
	   );

       if (hNewIcon)
       {
           SendMessage(hwnd, WM_SETICON, ICON_BIG, (LPARAM)hNewIcon);
           SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)hNewIcon);
       }
       else
       {
           MessageBox(NULL, L"Failed to load icon!", L"Error", MB_OK | MB_ICONERROR);
       }
    }

	void CRender::ProcessInput()
	{
		if (glfwGetKey(mp_Window, GLFW_KEY_R) == GLFW_PRESS && (reinterpret_cast<Int*>(abxPtr ^ 0xA5A5A5A5)->get() == 1337))
		{
			rSlider += offsetSlider;
		}
		if (glfwGetKey(mp_Window, GLFW_KEY_E) == GLFW_PRESS && (reinterpret_cast<Int*>(abxPtr ^ 0xA5A5A5A5)->get() == 1337))
		{
			rSlider -= offsetSlider;
		}
		if (glfwGetKey(mp_Window, GLFW_KEY_G) == GLFW_PRESS && (reinterpret_cast<Int*>(abxPtr ^ 0xA5A5A5A5)->get() == 1337))
		{
			gSlider += offsetSlider;
		}
		if (glfwGetKey(mp_Window, GLFW_KEY_F) == GLFW_PRESS && (reinterpret_cast<Int*>(abxPtr ^ 0xA5A5A5A5)->get() == 1337))
		{
			gSlider -= offsetSlider;
		}
		if (glfwGetKey(mp_Window, GLFW_KEY_B) == GLFW_PRESS && (reinterpret_cast<Int*>(abxPtr ^ 0xA5A5A5A5)->get() == 1337))
		{
			bSlider += offsetSlider;
		}
		if (glfwGetKey(mp_Window, GLFW_KEY_V) == GLFW_PRESS && (reinterpret_cast<Int*>(abxPtr ^ 0xA5A5A5A5)->get() == 1337))
		{
			bSlider -= offsetSlider;
		}
	}

	void CRender::UpdateView()
	{
		int widthFromBuffer;
		int heightFromBuffer;
		glfwGetFramebufferSize(mp_Window, &widthFromBuffer, &heightFromBuffer);
		float aspect = (float)widthFromBuffer / (float)heightFromBuffer;

		float zoomPerc = zoomSlider / 100;
		float orthoHeight = 1 / zoomPerc;
		float orthoWidth = orthoHeight * aspect;

		glm::mat4 Projection = glm::ortho(
			-orthoWidth, orthoWidth,
			-orthoHeight, orthoHeight,
			0.1f, 10.0f
		);

		glm::mat4 View = glm::lookAt(
			glm::vec3(0.0f, 0.0f, 1.0f),   // Camera position
			glm::vec3(0.0f, 0.0f, 0.0f),   // Look at origin (center of quad)
			glm::vec3(0.0f, 1.0f, 0.0f)    // Up vector
		);

		glm::mat4 Model = glm::mat4(1.0f);

		Model = glm::translate(Model, glm::vec3(0.35f + xOffsetSlider, yOffsetSlider, 0));

		Model = glm::rotate(
			Model,
			glm::radians(modelRotationInDegrees),
			glm::vec3(0.0f, 0.0f, 1.0f)
		);

		glm::mat4 MVP = Projection * View * Model;

		GLuint mvpLoc = glGetUniformLocation(alienProgramID, "MVP");
		if (mvpLoc != -1)
		{
			glUniformMatrix4fv(mvpLoc, 1, GL_FALSE, glm::value_ptr(MVP));
		}
		else
		{
			std::cerr << "MVP uniform not found!" << std::endl;
		}

		glViewport(0, 0, widthFromBuffer, heightFromBuffer);
	}

	void CRender::DoUI()
	{
		int widthFramebuffer;
		int heightFramebuffer;
		glfwGetFramebufferSize(mp_Window, &widthFramebuffer, &heightFramebuffer);

		//
		// borders
		//
		ImVec2 p_min = ImVec2(0, 19); // top-left
		ImVec2 p_max = ImVec2(271, static_cast<float>(heightFramebuffer)); // top-right
		ImU32 color = IM_COL32(36, 36, 36, 255);
		ImGui::GetBackgroundDrawList()->AddRectFilled(p_min, p_max, color);

		//
		// options
		//
		if (showOptions)
		{
			ImGui::SetNextWindowPos(ImVec2(10, 30));
			ImGui::SetNextWindowSize(ImVec2(250, 360));
			ImGui::Begin("Image Options");
			ImGui::Text("Color mix");
			ImGui::Checkbox("Use Red", &rSliderChecked);
			ImGui::SliderFloat("Red", &rSlider, 0.0f, 20.0f);
			ImGui::Checkbox("Use Green", &gSliderChecked);
			ImGui::SliderFloat("Green", &gSlider, 0.0f, 20.0f);
			ImGui::Checkbox("Use Blue", &bSliderChecked);
			ImGui::SliderFloat("Blue", &bSlider, 0.0f, 20.0f);
			ImGui::Text("Zoom");
			ImGui::SliderFloat("Zoom", &zoomSlider, 1.0f, 500.0f);
			ImGui::Text("Offset");
			ImGui::SliderFloat("X Offset", &xOffsetSlider, -2.0f, 2.0f);
			ImGui::SliderFloat("Y Offset", &yOffsetSlider, -1.5f, 1.5f);
			ImGui::Spacing();
			ImGui::Spacing();
			if (ImGui::Button("Reset all values"))
			{
				ResetSliderValues();
			}
			ImGui::Spacing();
			ImGui::Spacing();
			ImGui::Spacing();
			if (ImGui::Button("Open an Image..."))
			{
				OpenFileDialog();
			}
			ImGui::End();
		}

		//
		// Help/About dialog
		//
		if (showAboutDialog)
		{
			float screenWidth = ImGui::GetIO().DisplaySize.x;
			float windowSz = 330;
			ImGui::SetNextWindowPos(ImVec2(screenWidth * 0.5f - windowSz * 0.5f, 40));
			ImGui::OpenPopup("About Dialog");
			if (ImGui::BeginPopupModal("About Dialog", NULL, ImGuiWindowFlags_AlwaysAutoResize))
			{
				ImGui::Text("@copyright ImagePro tecnologies.\n");
				ImGui::Text("Version: 2.0.1 (64bit)\n");
				ImGui::Spacing();
				float windowWidth = ImGui::GetWindowSize().x;
				float buttonWidth = 120.0f;
				ImGui::SetCursorPosX((windowWidth - buttonWidth) * 0.5f);
				if (ImGui::Button("OK", ImVec2(buttonWidth, 0)))
				{
					ImGui::CloseCurrentPopup();
					showAboutDialog = false;
				}

				ImGui::EndPopup();
			}
		}

		//
		// Help>License dialog
		//
		if (showLicenseDialog)
		{
			float screenWidth = ImGui::GetIO().DisplaySize.x;
			float windowSz = 330;
			ImGui::SetNextWindowPos(ImVec2(screenWidth * 0.5f - windowSz * 0.5f, 40));
			ImGui::OpenPopup("License Dialog");
			if (ImGui::BeginPopupModal("License Dialog", NULL, ImGuiWindowFlags_AlwaysAutoResize))
			{
				ImGui::Text("To get information about the license of this sofware,\n");
				ImGui::Text("please read the full license terms in the button bellow.");
				ImGui::Separator();

				float windowWidth = ImGui::GetWindowSize().x;
				float buttonWidth = 120.0f;
				ImGui::SetCursorPosX((windowWidth - buttonWidth) * 0.5f);
				if (ImGui::Button("Click to open", ImVec2(buttonWidth, 0)))
				{
					ImGui::CloseCurrentPopup();
					showLicenseDialog = false;
					system("imagepro_license.txt");
				}

				ImGui::EndPopup();
			}
		}

		//
		// Help>Terms of use
		//
		if (showTermsOfUse)
		{
			float screenWidth = ImGui::GetIO().DisplaySize.x;
			float windowSz = 330;
			ImGui::SetNextWindowPos(ImVec2(screenWidth * 0.5f - windowSz * 0.5f, 40));
			ImGui::OpenPopup("Terms of use");
			if (ImGui::BeginPopupModal("Terms of use", NULL, ImGuiWindowFlags_AlwaysAutoResize))
			{
				ImGui::Text("To get information about the terms of use of this sofware,\n");
				ImGui::Text("please read the full terms in the button bellow.\n");
				ImGui::Text("Alert: it is forbidden to use photos of person < 18 years!\n");
				ImGui::Separator();

				float windowWidth = ImGui::GetWindowSize().x;
				float buttonWidth = 120.0f;
				ImGui::SetCursorPosX((windowWidth - buttonWidth) * 0.5f);
				if (ImGui::Button("Click to open", ImVec2(buttonWidth, 0)))
				{
					ImGui::CloseCurrentPopup();
					showTermsOfUse = false;
					system("imagepro_terms_of_use.txt");
				}

				ImGui::EndPopup();
			}
		}

		//
		// initial license alerts
		//
		{
			if ((reinterpret_cast<Int*>(abxPtr ^ 0xA5A5A5A5)->get() != 1337))
			{
				ImGui::SetNextWindowPos(ImVec2(283, 30));
				ImGui::SetNextWindowSize(ImVec2(380, 80));
				ImGui::Begin("Activate");
				if (ImGui::Button("Click on this button open Website and activate..."))
				{
					std::string endpoint = License::getInstance().getSiteEndpoint();
					OpenURL(endpoint);
				}

				if (ImGui::Button("Already Activated? Click here to check License."))
				{
					License::getInstance().checkLicense(abxPtr, true);
					showLicenseAlertChecking = true;
				}

				ImGui::End();
			}

			if (showLicenseAlertChecking && !License::getInstance().isLicenseChecked())
			{
				ImGui::OpenPopup("Checking License Alert");
			}

			if (ImGui::BeginPopupModal("Checking License Alert", NULL, ImGuiWindowFlags_AlwaysAutoResize))
			{
				ImGui::Text("Checking the license, wait a moment...");
				ImGui::EndPopup();
			}

			if (showLicenseAlertChecking && License::getInstance().isLicenseChecked())
			{
				ImGui::OpenPopup("License Alert");
			}

			if (ImGui::BeginPopupModal("License Alert", NULL, ImGuiWindowFlags_AlwaysAutoResize))
			{
				if ((reinterpret_cast<Int*>(abxPtr ^ 0xA5A5A5A5)->get() != 1337))
				{
					ImGui::Text("Your license is not valid.\nCheck if you have enabled it.\nOtherwise send us support message.");
				}
				else
				{
					ImGui::Text("Your license is ok.\nClick ok to continue.\nThank you!");
				}
				ImGui::Separator();

				float windowWidth = ImGui::GetWindowSize().x;
				float buttonWidth = 120.0f;
				ImGui::SetCursorPosX((windowWidth - buttonWidth) * 0.5f);
				if (ImGui::Button("OK", ImVec2(buttonWidth, 0)))
				{
					ImGui::CloseCurrentPopup();
					showLicenseAlertChecking = false;
					License::getInstance().setLicenseChecked(false);
				}

				ImGui::EndPopup();
			}
		}

		//
		// image age checking
		//
		if (!imagepro::AgeDetection::GetInstance().isCurrImageAdultPerson())
		{
			float screenWidth = ImGui::GetIO().DisplaySize.x;
			float windowSz = 330;
			ImGui::SetNextWindowPos(ImVec2(screenWidth - windowSz - 20, 40));
			ImGui::SetNextWindowSize(ImVec2(windowSz, 85));
			bool adultAlertShowFlag;
			ImGui::Begin("Alert!", &adultAlertShowFlag);
			ImGui::Text("Image appear to be from a person < 18 years;\nThis is forbidden with this software;\nIf false positive please ignore this message.");
			ImGui::End();
		}

		//
		// color picker
		//
		if (showColorPicker)
		{
			float screenWidth = ImGui::GetIO().DisplaySize.x;
			float windowSz = 300;
			ImGui::SetNextWindowPos(ImVec2(screenWidth * 0.5f - windowSz * 0.5f, 40));
			ImGui::SetNextWindowSize(ImVec2(windowSz, 60));
			ImGui::Begin("Color Picker", &showColorPicker);
			if (ImGui::ColorEdit4("Pick Color", (float*)&colorPicker))
			{
				glClearColor(colorPicker.x, colorPicker.y, colorPicker.z, 0.0f);
			}
			ImGui::End();
		}

		//
		// menu bar
		//

		if (ImGui::BeginMainMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				if (ImGui::MenuItem("Open a New Image"))
				{
					OpenFileDialog();
				}

				if (ImGui::MenuItem("Save Current Image..."))
				{
					saveImageToFileFlag = true;
					saveImageToFileName = OpenSaveFileDialog();
				}

				ImGui::Separator();

				if (ImGui::MenuItem("Reset Image Options Values"))
				{
					ResetSliderValues();
				}

				ImGui::Separator();

				if (ImGui::MenuItem("Exit"))
				{
					programRunning = false;
				}
				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Edit"))
			{
				if (ImGui::MenuItem("Copy"))
				{
					copyCurrentImageToClipboardFlag = true;
				}

				ImGui::Separator();

				if (ImGui::MenuItem("RotateLeft"))
				{
					modelRotationInDegrees += 90;
				}

				if (ImGui::MenuItem("RotateRight"))
				{
					modelRotationInDegrees -= 90;
				}
				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Window"))
			{
				//if (ImGui::MenuItem("Undo")) {}

				//if (ImGui::MenuItem("Redo")) {}

				if (showOptions)
				{
					if (ImGui::MenuItem("Hide Options"))
					{
						showOptions = false;
					}
				}
				else
				{
					if (ImGui::MenuItem("Show Options"))
					{
						showOptions = true;
					}
				}

				ImGui::Separator();
				if (ImGui::MenuItem("Change Background"))
				{
					showColorPicker = true;
				}
				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Help"))
			{
				if (ImGui::MenuItem("About"))
				{
					showAboutDialog = true;
				}

				if (ImGui::MenuItem("License"))
				{
					showLicenseDialog = true;
				}


				if (ImGui::MenuItem("Terms of Use"))
				{
					showTermsOfUse = true;
				}

				ImGui::Separator();

				if (ImGui::MenuItem("Send a Support Message"))
				{
					OpenURL(License::getInstance().getSiteSupportEndpoint());
				}

				ImGui::Separator();

				if (ImGui::MenuItem("Open Website to Activate"))
				{
					OpenURL(License::getInstance().getSiteEndpoint());
				}

				if (ImGui::MenuItem("Check License"))
				{
					License::getInstance().checkLicense(abxPtr, true);
					showLicenseAlertChecking = true;
				}
				ImGui::EndMenu();
			}

			ImGui::EndMainMenuBar();
		}


		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	}

	void CRender::MainLoop()
	{
		do
		{
			//
			// input
			//
			ProcessInput();

			//
			// clear the screen
			//
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			//
			// ImGui
			//

			ImGui_ImplOpenGL3_NewFrame();
			ImGui_ImplGlfw_NewFrame();
			ImGui::NewFrame();

			//
			// bind texture
			//

			// Bind our texture in Texture Unit 0
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, mi_Texture);

			UpdateView();

			//
			// use shader and set shader params
			//
			{
				glUseProgram(alienProgramID);

				GLint freqLocation = glGetUniformLocation(alienProgramID, "freq");
				GLint phaseShiftLocation = glGetUniformLocation(alienProgramID, "phaseshift");
				GLint keepLocation = glGetUniformLocation(alienProgramID, "keep");

				glUniform3f(freqLocation, (reinterpret_cast<Int*>(abxPtr ^ 0xA5A5A5A5)->get() == 1337) ? rSlider : 0,
					(reinterpret_cast<Int*>(abxPtr ^ 0xA5A5A5A5)->get() == 1337) ? gSlider : 0,
					(reinterpret_cast<Int*>(abxPtr ^ 0xA5A5A5A5)->get() == 1337) ? bSlider : 0);
				glUniform3f(phaseShiftLocation, 0, 0, 0);
				glUniform3f(keepLocation, rSliderChecked && (reinterpret_cast<Int*>(abxPtr ^ 0xA5A5A5A5)->get() == 1337) ? 1.0f : 0,
					gSliderChecked && (reinterpret_cast<Int*>(abxPtr ^ 0xA5A5A5A5)->get() == 1337) ? 1.0f : 0,
					bSliderChecked && (reinterpret_cast<Int*>(abxPtr ^ 0xA5A5A5A5)->get() == 1337) ? 1.0f : 0);

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
				glm::vec2 offset(xOffsetSliderShader, yOffsetSliderShader);

				//
				// Send uniforms to the shader
				//
				glUniform1f(zoomLoc, 1 - zoomSliderShader / 20.0f);
				glUniform2fv(offsetLoc, 1, &offset[0]);

				glUniform1i(glGetUniformLocation(alienProgramID, "inputTexture"), 0);
			}

			//
			// 1rst attribute buffer : vertices
			//
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

			//
			// 2nd attribute buffer : UVs
			//
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

			//
			// draw the triangles (our quad)
			//
			glDrawArrays(GL_TRIANGLES, 0, 12 * 3); // 12*3 indices starting at 0 -> 12 triangles

			glDisableVertexAttribArray(0);
			glDisableVertexAttribArray(1);

			//
			// clipboard
			//
			if (copyCurrentImageToClipboardFlag)
			{
				copyCurrentImageToClipboardFlag = false;
				int widthFramebuffer;
				int heightFramebuffer;
				glfwGetFramebufferSize(mp_Window, &widthFramebuffer, &heightFramebuffer);
				auto pixels = ReadOpenGLFramebufferToBGRA(widthFramebuffer, heightFramebuffer);
				CopyPixelsToClipboard(widthFramebuffer, heightFramebuffer, pixels);
			}

			//
			// save image to file
			//
			if (saveImageToFileFlag)
			{
				saveImageToFileFlag = false;
				int widthFramebuffer, heightFramebuffer;
				glfwGetFramebufferSize(mp_Window, &widthFramebuffer, &heightFramebuffer);
				auto pixels = ReadOpenGLFramebufferToBGRA(widthFramebuffer, heightFramebuffer);
				SavePixelsToFile(widthFramebuffer, heightFramebuffer, pixels, saveImageToFileName);
			}

			//
			// render UI
			//

			DoUI();

			//
			// Swap buffers
			//
			glfwSwapBuffers(mp_Window);
			glfwPollEvents();

			//
			// license
			//
			{
				License::getInstance().checkLicense(abxPtr, false);
			}

		} while (programRunning && glfwWindowShouldClose(mp_Window) == 0);
	}


}
