/**
*	@file OffscreenBuffer.cpp
*/
#include"OffscreenBuffer.h"

/**
*	�I�t�X�N���[���o�b�t�@���쐬����
*
*	@param	w �I�t�X�N���[���o�b�t�@�̕��@(�s�N�Z���P��)
*	@param	h �I�t�X�N���[���o�b�t�@�̍���()�s�N�Z���P��
*
*	@return	�쐬�����I�t�X�N���[���o�b�t�@�ւ̃|�C���^
*/
OffscreenBufferPtr OffscreenBuffer::Create(int w, int h) {
	struct  Impl : OffscreenBuffer{
		Impl() {}
		~Impl() {}
	};
	OffscreenBufferPtr offscreen = std::make_shared<Impl>();
	if (!offscreen) {
		return offscreen;
	}

	//�e�N�X�`�����쐬
	offscreen->tex = Texture::Create(w, h, GL_RGBA8, GL_RGBA, nullptr);

	//�[�x�o�b�t�@���쐬
	glGenRenderbuffers(1, &offscreen->depthBuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, offscreen->depthBuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, w, h);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	//�e�N�X�`���Ɛ[�x�o�b�t�@���t���[���o�b�t�@�Ƃ��ē���
	glGenFramebuffers(1, &offscreen->frameBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, offscreen->frameBuffer);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, offscreen->depthBuffer);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, offscreen->tex->Id(), 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);


	return offscreen;
}

/**
*	�f�X�g���N�^
*/
OffscreenBuffer::~OffscreenBuffer() {
	if (frameBuffer) {
		glDeleteFramebuffers(1, &frameBuffer);
	}
	if (depthBuffer) {
		glDeleteRenderbuffers(1, &depthBuffer);
	}
}