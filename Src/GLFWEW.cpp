/**
* @file GLFWEW.cpp
*/

#include"GLFWEW.h"
#include<iostream>

///GLFW��GLEW�����b�v���邽�߂̖��O���
namespace GLFWEW {
	/**
	* GLFW ����̃G���[�񍐂���������
	*
	* @param error	�G���[�ԍ�
	* @param desc	�G���[�̓��e
	*/
	void ErrorCallback(int error, const char* desc) {
		std::cerr << "ERROR: " << desc << std::endl;
	}

	/**
	* �V���O���g���C���X�^���X���擾����
	*
	* @return Window �̃V���O���g���C���X�^���X
	*/
	Window& Window::Instance() {
		static Window instance;
		return instance;
	}

	//�R���X�g���N�^
	Window::Window() :isGLFWInitialized(false), isInitialized(false), window(nullptr) {

	}

	//�f�X�g���N�^
	Window::~Window() {
		if (isGLFWInitialized) {
			glfwTerminate();
		}
	}

	/**
	* GLFW/GLEW �̏�����
	*
	* @param w		�E�B���h�E�̕`��͈͂̕�(�s�N�Z��)
	* @param h		�E�B���h�E�̕`��͈͂̍���(�s�N�Z��)
	* @param title	�E�B���h�E�^�C�g��(UTF-8 �� 0 �I�[������)
	*
	* @retval true	����������
	* @retval false	���������s
	*/
	bool Window::Init(int w, int h, const char* title) {
		//GLFW�̏�����
		if (isInitialized) {
			std::cerr << "ERROR: GLFWEW �͊��ɑ��݂��Ă��܂��B" << std::endl;
			return false;
		}
		if (!isGLFWInitialized) {
			glfwSetErrorCallback(ErrorCallback);
			if (glfwInit() != GL_TRUE) {
				return false;
			}
			isGLFWInitialized = true;
		}
		if (!window) {
			window = glfwCreateWindow(w, h, title, nullptr, nullptr);
			if (!window) {
				return false;
			}
			glfwMakeContextCurrent(window);
		}
		//GLEW�̏�����
		if (glewInit() != GLEW_OK) {
			std::cerr << "ERROR: GLEW�̏������Ɏ��s���܂����B" << std::endl;
			return false;
		}

		//�`��f�o�C�X�ƑΉ��o�[�W�������o��
		const GLubyte* renderer = glGetString(GL_RENDERER);
		std::cout << "Renderer: " << renderer << std::endl;
		const GLubyte* version = glGetString(GL_VERSION);
		std::cout << "Version: " << version << std::endl;
		isInitialized = true;
		return true;
	}

	/**
	* �E�B���h�E�����ׂ������ׂ�
	*
	* @retval true	����
	* @retval false	���Ȃ�
	*/
	bool Window::ShouldClose() const {
		return glfwWindowShouldClose(window) != 0;
	}

	//�t�����g�o�b�t�@�ƃo�b�N�o�b�t�@��؂�ւ���
	void Window::SwapBuffers() const {
		glfwPollEvents();
		glfwSwapBuffers(window);
	}

	const GamePad& Window::GetGamePad() const {
		return gamePad;
	}

	/**
	*	�W���C�X�e�B�b�N�̃A�i���O���͑��u ID
	+
	*	@note XBOX360 �R���g���[���[�
	*/
	enum GLFWAXESID {
		GLFWAXESID_LeftX,	///< ���X�e�B�b�N�� x ��
		GLFWAXESID_LeftY,	///< ���X�e�B�b�N�� y ��
		GLFWAXESID_BackX,	///< �A�i���O�g���K�[
		GLFWAXESID_RightX,	///< �E�X�e�B�b�N�� x ��
		GLFWAXESID_RightY,	///< �E�X�e�B�b�N�� y ��
	};

	/**
	*	�W���C�X�e�B�b�N�̃f�W�^�����͑��u ID
	+
	*	@note XBOX360 �R���g���[���[�
	*/
	enum GLFWBUTTONID {
		GLFWAXESID_A,	///< A �{�^��
		GLFWAXESID_B,	///< B �{�^��
		GLFWAXESID_X,	///< X �{�^��
		GLFWAXESID_Y,	///< Y �{�^��
		GLFWAXESID_L,	///< L �{�^��
		GLFWAXESID_R,	///< R �{�^��
		GLFWAXESID_Back,	///< Back �{�^��
		GLFWAXESID_Start,	///< Start �{�^��
		GLFWAXESID_LThumb,	///< ���X�e�B�b�N�{�^��
		GLFWAXESID_RThumb,	///< �E�X�e�B�b�N�{�^��
		GLFWAXESID_Up,		///< ��L�[
		GLFWAXESID_Right,	///< �E�L�[
		GLFWAXESID_Down,	///< ���L�[
		GLFWAXESID_Left,	///< ���L�[
	};

	/**
	*	�Q�[���p�b�h�̏�Ԃ��X�V����
	*/
	void Window::UpdateGamePad() {
		const uint32_t prevButtons = gamePad.buttons;
		int axesCount, buttonCount;
		const float* axes = glfwGetJoystickAxes(GLFW_JOYSTICK_1, &axesCount);
		const uint8_t* buttons = glfwGetJoystickButtons(GLFW_JOYSTICK_1, &buttonCount);
		if (axes && buttons && axesCount >= 2 && buttonCount >= 8) {
			gamePad.buttons &= ~(GamePad::DPAD_UP | GamePad::DPAD_DOWN | GamePad::DPAD_LEFT | GamePad::DPAD_RIGHT);
			static const float threshould = 0.3f;
			if (axes[GLFWAXESID_LeftY] >= threshould) {
				gamePad.buttons |= GamePad::DPAD_UP;
			}else if (axes[GLFWAXESID_LeftY] <= -threshould) {
				gamePad.buttons |= GamePad::DPAD_DOWN;
			}
			if (axes[GLFWAXESID_LeftX] >= threshould) {
				gamePad.buttons |= GamePad::DPAD_LEFT;
			}
			else if (axes[GLFWAXESID_LeftX] <= -threshould) {
				gamePad.buttons |= GamePad::DPAD_RIGHT;
			}
			static const struct {
				int glfwCode;
				uint32_t gamepadCode;
			} keyMap[] ={
				{ GLFWAXESID_A, GamePad::A },
				{ GLFWAXESID_B, GamePad::B },
				{ GLFWAXESID_X, GamePad::X },
				{ GLFWAXESID_Y, GamePad::Y },
				{ GLFWAXESID_Start, GamePad::START },
			};
			for (const auto& e : keyMap) {
				if (buttons[e.glfwCode] == GLFW_PRESS) {
					gamePad.buttons |= e.gamepadCode;
				} else if (buttons[e.glfwCode] == GLFW_RELEASE) {
					gamePad.buttons &= ~e.gamepadCode;
				}
			}
		} else {
			static const struct {
				int glfwCode;
				uint32_t gamepadCode;
			} keyMap[] = {
				{ GLFW_KEY_UP, GamePad::DPAD_UP },
				{ GLFW_KEY_DOWN, GamePad::DPAD_DOWN },
				{ GLFW_KEY_LEFT, GamePad::DPAD_LEFT },
				{ GLFW_KEY_RIGHT, GamePad::DPAD_RIGHT },
				{ GLFW_KEY_ENTER, GamePad::START },
				{ GLFW_KEY_A, GamePad::A },
				{ GLFW_KEY_S, GamePad::B },
				{ GLFW_KEY_Z, GamePad::X },
				{ GLFW_KEY_X, GamePad::Y },
			};
			for (const auto& e : keyMap) {
				const int key = glfwGetKey(window, e.glfwCode);
				if (key == GLFW_PRESS) {
					gamePad.buttons |= e.gamepadCode;
				}
				else if (key == GLFW_RELEASE) {
					gamePad.buttons &= ~e.gamepadCode;
				}
			}
		}
		gamePad.buttonDown = gamePad.buttons & prevButtons;
	}

}

