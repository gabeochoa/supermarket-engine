#pragma once

#include "pch.hpp"

struct VertexBuffer {
    virtual ~VertexBuffer() {}
    virtual void bind() const = 0;
    virtual void unbind() const = 0;
    static VertexBuffer* create(float* verts, int size);
};

struct IndexBuffer {
    virtual ~IndexBuffer() {}
    virtual void bind() const = 0;
    virtual void unbind() const = 0;
    virtual unsigned int getCount() const { return count; }
    unsigned int count;

    static IndexBuffer* create(unsigned int* i_s, unsigned int count);
};

struct OpenGLVertexBuffer : public VertexBuffer {
    unsigned int rendererID;
    OpenGLVertexBuffer(float* verts, unsigned int size) {
        glGenBuffers(1, &rendererID);
        glBindBuffer(GL_ARRAY_BUFFER, rendererID);
        glBufferData(GL_ARRAY_BUFFER, size, verts, GL_STATIC_DRAW);
    }
    virtual ~OpenGLVertexBuffer() { glDeleteBuffers(1, &rendererID); }
    virtual void bind() const override {
        glBindBuffer(GL_ARRAY_BUFFER, rendererID);
    }
    virtual void unbind() const override { glBindBuffer(GL_ARRAY_BUFFER, 0); }
};

struct OpenGLIndexBuffer : public IndexBuffer {
    unsigned int rendererID;
    OpenGLIndexBuffer(unsigned int* i_s, unsigned int count) {
        this->count = count;
        glGenBuffers(1, &rendererID);
        glBindBuffer(GL_ARRAY_BUFFER, rendererID);
        glBufferData(GL_ARRAY_BUFFER, count * sizeof(unsigned int), i_s,
                     GL_STATIC_DRAW);
    }
    virtual ~OpenGLIndexBuffer() { glDeleteBuffers(1, &rendererID); }
    virtual void bind() const override {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, rendererID);
    }
    virtual void unbind() const override {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }
};

