#include "buffer.h"

#include "pch.hpp"

VertexBuffer* VertexBuffer::create(float* verts, int size) {
    return new OpenGLVertexBuffer(verts, size);
}

IndexBuffer* IndexBuffer::create(unsigned int* i_s, unsigned int count) {
    return new OpenGLIndexBuffer(i_s, count);
}
