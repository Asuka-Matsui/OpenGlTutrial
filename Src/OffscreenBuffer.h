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
*	�I�t�X�N���[���o�b�t�@
*/
class OffscreenBuffer {
public:
	static OffscreenBufferPtr Create(int w, int h);
	GLuint GetFrameBuffer() const { return frameBuffer; }	///< �t���[���o�b�t�@���擾����
	GLuint GetTextur() const { return tex->Id(); }	///< �t���[���o�b�t�@�p�e�N�X�`�����擾����

private:
	OffscreenBuffer() = default;
	OffscreenBuffer(const OffscreenBuffer&) = default;
	OffscreenBuffer& operator = (const OffscreenBuffer&) = default;
	~OffscreenBuffer();


private:
	TexturePtr tex;	///< �t���[���o�b�t�@�p�e�N�X�`��
	GLuint depthBuffer = 0;	///< �[�x�o�b�t�@�I�u�W�F�N�g
	GLuint frameBuffer = 0;	///< �t���[���o�b�t�@�I�u�W�F�N�g

};