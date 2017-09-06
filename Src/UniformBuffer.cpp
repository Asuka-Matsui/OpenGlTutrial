//�C���N���[�h
#include "UniformBuffer.h"
#include <iostream>

/**
*   UniformBuffer ���쐬����
*
*   @param size         �o�b�t�@�̃o�C�g�T�C�Y
*   @param bindingPoint �o�b�t�@�����蓖�Ă�o�C���f�B���O�E�|�C���g
*   @param name         �o�b�t�@��
*
*   @return �쐬���� UniformBuffer �ւ̃|�C���^
*/
UniformBufferPtr UniformBuffer::Create(GLsizeiptr size, GLuint bindingPoint, const char* name){
    struct Impl : UniformBuffer{ Impl(){} ~Impl(){} };
    UniformBufferPtr p = std::make_shared<Impl>();
    if(!p){
        std::cerr << "ERROR: UBO" << name << "�̍쐬�Ɏ��s" << std::endl;
        return {};
    }

    glGenBuffers(1, &p->ubo);
    glBindBuffer(GL_UNIFORM_BUFFER, p->ubo);
	glBufferData(GL_UNIFORM_BUFFER, size, nullptr, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, bindingPoint, p->ubo);
    const GLenum result = glGetError();
    if(result != GL_NO_ERROR){
        std::cerr << "ERROR: UBO" << name <<  "'�̍쐬�Ɏ��s" << std::endl;
        return {};
    }

    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    p->size = size;
    p->bindingPoint = bindingPoint;
    p->name = name;

    return p;
}

/**
*   �f�X�g���N�^
*/
UniformBuffer::~UniformBuffer(){
    if(ubo){
        glDeleteBuffers(1, &ubo);
    }
}

/**
*  UniformBuffer �Ƀf�[�^��]������ 
*
*   @param data     �]������f�[�^�ւ̃|�C���^
*   @param offset   �]����̃o�C�g�I�t�Z�b�g
*   @param size     �]������o�C�g��
*
*   @retval true    �]������
*   @retval false   �]�����s
*
*   offset �� size �̗����� 0 �̏ꍇ�A�o�b�t�@�T�C�Y = UBO �T�C�Y�Ƃ��ē]�������
*/
bool UniformBuffer::BufferSubData(const GLvoid* data, GLintptr offset, GLsizeiptr size ){
    if(offset + size > this->size){
        std::cerr << "ERROR (" << name << "):�]���͈͂��o�b�t�@�T�C�Y�𒴂��Ă��܂�(Buffer =" <<
        this->size << ", offset =" << offset << ", size =" << size << ")" <<std::endl;
        return false;
    }
    if(offset == 0 && size == 0){
        size = this->size;
    }
    glBindBuffer(GL_UNIFORM_BUFFER, ubo);
    glBufferSubData(GL_UNIFORM_BUFFER, offset, size, data);
    return true;
}

/**
*  �w�肳�ꂽ�͈͂��o�C���f�B���O�E�|�C���g�Ɋ��蓖�Ă�
*
*   @param offset ���蓖�Ă�͈͂̃o�C�g�I�t�Z�b�g
*   @param size   ���蓖�Ă�͈͂̃o�C�g��
*/
void UniformBuffer::BufferRange(GLintptr offset, GLsizeiptr size) const {
	glBindBufferRange(GL_UNIFORM_BUFFER, bindingPoint, ubo, offset, size);
}

/**
*  UBO ���V�X�e���������Ƀ}�b�v����
*
*   @return �}�b�v�����|�C���^�ւ̃|�C���^
*/
void* UniformBuffer::MapBuffer() const {
	glBindBuffer(GL_UNIFORM_BUFFER, ubo);
	return glMapBufferRange(GL_UNIFORM_BUFFER, 0, size, GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
}

/**
*  �o�b�t�@�̊��蓖�Ă���������
*/
void UniformBuffer::UnmapBuffer() const {
	glUnmapBuffer(GL_UNIFORM_BUFFER);
}