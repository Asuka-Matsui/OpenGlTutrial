/**
* @file GLFWEW.cpp
*/

#include"GLFWEW.h"
#include<iostream>

///GLFWとGLEWをラップするための名前空間
namespace GLFWEW {
	/**
	* GLFW からのエラー報告を処理する
	*
	* @param error	エラー番号
	* @param desc	エラーの内容
	*/
	void ErrorCallback(int error, const char* desc) {
		std::cerr << "ERROR: " << desc << std::endl;
	}

	/**
	* シングルトンインスタンスを取得する
	*
	* @return Window のシングルトンインスタンス
	*/
	Window& Window::Instance() {
		static Window instance;
		return instance;
	}

	//コンストラクタ
	Window::Window() :isGLFWInitialized(false), isInitialized(false), window(nullptr) {

	}

	//デストラクタ
	Window::~Window() {
		if (isGLFWInitialized) {
			glfwTerminate();
		}
	}

	/**
	* GLFW/GLEW の初期化
	*
	* @param w		ウィンドウの描画範囲の幅(ピクセル)
	* @param h		ウィンドウの描画範囲の高さ(ピクセル)
	* @param title	ウィンドウタイトル(UTF-8 の 0 終端文字列)
	*
	* @retval true	初期化成功
	* @retval false	初期化失敗
	*/
	bool Window::Init(int w, int h, const char* title) {
		//GLFWの初期化
		if (isInitialized) {
			std::cerr << "ERROR: GLFWEW は既に存在しています。" << std::endl;
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
		//GLEWの初期化
		if (glewInit() != GLEW_OK) {
			std::cerr << "ERROR: GLEWの初期化に失敗しました。" << std::endl;
			return false;
		}

		//描画デバイスと対応バージョンを出力
		const GLubyte* renderer = glGetString(GL_RENDERER);
		std::cout << "Renderer: " << renderer << std::endl;
		const GLubyte* version = glGetString(GL_VERSION);
		std::cout << "Version: " << version << std::endl;
		isInitialized = true;
		return true;
	}

	/**
	* ウィンドウを閉じるべきか調べる
	*
	* @retval true	閉じる
	* @retval false	閉じない
	*/
	bool Window::ShouldClose() const {
		return glfwWindowShouldClose(window) != 0;
	}

	//フロントバッファとバックバッファを切り替える
	void Window::SwapBuffers() const {
		glfwPollEvents();
		glfwSwapBuffers(window);
	}

	const GamePad& Window::GetGamePad() const {
		return gamePad;
	}

	/**
	*	ジョイスティックのアナログ入力装置 ID
	+
	*	@note XBOX360 コントローラー基準
	*/
	enum GLFWAXESID {
		GLFWAXESID_LeftX,	///< 左スティックの x 軸
		GLFWAXESID_LeftY,	///< 左スティックの y 軸
		GLFWAXESID_BackX,	///< アナログトリガー
		GLFWAXESID_RightX,	///< 右スティックの x 軸
		GLFWAXESID_RightY,	///< 右スティックの y 軸
	};

	/**
	*	ジョイスティックのデジタル入力装置 ID
	+
	*	@note XBOX360 コントローラー基準
	*/
	enum GLFWBUTTONID {
		GLFWAXESID_A,	///< A ボタン
		GLFWAXESID_B,	///< B ボタン
		GLFWAXESID_X,	///< X ボタン
		GLFWAXESID_Y,	///< Y ボタン
		GLFWAXESID_L,	///< L ボタン
		GLFWAXESID_R,	///< R ボタン
		GLFWAXESID_Back,	///< Back ボタン
		GLFWAXESID_Start,	///< Start ボタン
		GLFWAXESID_LThumb,	///< 左スティックボタン
		GLFWAXESID_RThumb,	///< 右スティックボタン
		GLFWAXESID_Up,		///< 上キー
		GLFWAXESID_Right,	///< 右キー
		GLFWAXESID_Down,	///< 下キー
		GLFWAXESID_Left,	///< 左キー
	};

	/**
	*	ゲームパッドの状態を更新する
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

