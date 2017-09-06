/**
* @file GLFWEW.h
*/
#pragma once

#include<GL/glew.h>
#include<GLFW/glfw3.h>
#include "GamePad.h"

namespace GLFWEW {
	/**
	* GLFW ����̃G���[�񍐂���������
	*
	* @param error	�G���[�ԍ�
	* @param desc	�G���[�̓��e
	*/
	void ErrorCallback(int error, const char* desc);

	/**
	* GLFW �� GLEW �̃��b�p�[�N���X
	*/
	class Window {
	public:
		static Window& Instance();
		bool Init(int w, int h, const char* title);
		bool ShouldClose() const;
		void SwapBuffers() const;
		const GamePad& GetGamePad() const;
		void UpdateGamePad();

	private:
		Window();
		~Window();
		Window(const Window&) = delete;
		Window& operator = (const Window&) = delete;

		bool isGLFWInitialized;
		bool isInitialized;
		GLFWwindow* window;
		GamePad gamePad;
	};
}