#pragma once
//インクルード
#include <GL/glew.h>
#include <string>
#include <memory>

class UniformBuffer;

typedef std::shared_ptr<UniformBuffer> UniformBufferPtr;    ///< UBO ポインタ

/**
*   UBO クラス
*/
class UniformBuffer{
public:
    static UniformBufferPtr Create(GLsizeiptr size, GLuint bindingPoint, const char* name);
    bool BufferSubData(const GLvoid* data, GLintptr offset = 0, GLsizeiptr size = 0);
	void BufferRange(GLintptr offset, GLsizeiptr size) const;
	void* MapBuffer() const;
	void UnmapBuffer() const;
	GLsizeiptr Size() const { return size; }

private:
    UniformBuffer() = default;
    ~UniformBuffer();
    UniformBuffer(const UniformBuffer&) = delete;
    UniformBuffer& operator=(const UniformBuffer&) = delete;

private:
    GLuint ubo = 0;
	GLsizeiptr size = 0;
    GLuint bindingPoint;
    std::string name;
};