/**
*	@file OffscreenBuffer.h
*/
#pragma once
#include"Texture.h"
#include<GL\glew.h>
#include<memory>

class OffscreenBuffer;
typedef std::shared_ptr<OffscreenBuffer> OffscreenBufferPtr;

/**
*	オフスクリーンバッファ
*/
class OffscreenBuffer {
public:
	static OffscreenBufferPtr Create(int w, int h);
	GLuint GetFrameBuffer() const { return frameBuffer; }	///< フレームバッファを取得する
	GLuint GetTextur() const { return tex->Id(); }	///< フレームバッファ用テクスチャを取得する

private:
	OffscreenBuffer() = default;
	OffscreenBuffer(const OffscreenBuffer&) = default;
	OffscreenBuffer& operator = (const OffscreenBuffer&) = default;
	~OffscreenBuffer();


private:
	TexturePtr tex;	///< フレームバッファ用テクスチャ
	GLuint depthBuffer = 0;	///< 深度バッファオブジェクト
	GLuint frameBuffer = 0;	///< フレームバッファオブジェクト

};