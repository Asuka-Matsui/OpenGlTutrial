#pragma once

#include<GL/glew.h>
#include<memory>

class Texture;
///�e�N�X�`���|�C���^
typedef std::shared_ptr<Texture> TexturePtr;

/**
*	�e�N�X�`���N���X
*/
class Texture{
public:
	static TexturePtr Create(int width, int height, GLenum iformat, GLenum formet, const void* data);
	static TexturePtr LoadFromFile(const char*);

	GLuint Id() const { return texId; }
	GLsizei Width() const { return width; }
	GLsizei Hieght() const { return height; }

private:
	Texture();
	~Texture();
	Texture(const Texture&) = delete;
	Texture& operator = (const Texture&) = delete;

	GLuint texId;
	int width;
	int height;
};