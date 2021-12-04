#pragma once

#include "pch.hpp"

enum class BufferType {
    None = 0,
    Float,
    Float2,
    Float3,
    Float4,
    Mat3,
    Mat4,
    Int,
    Int2,
    Int3,
    Int4,
    Bool
};

struct BufferElem {
    std::string name;
    BufferType type;
    uintptr_t offset;
    int size;
    bool normalized;

    BufferElem(const std::string& n, BufferType t, bool norm = false)
        : name(n),
          type(t),
          offset(0),
          size(getDataTypeSize(t)),
          normalized(norm) {}

    inline GLenum typeToOpenGLType() const {
        switch (type) {
            case BufferType::Bool:
                return GL_BOOL;
            case BufferType::Float:
            case BufferType::Float2:
            case BufferType::Float3:
            case BufferType::Float4:
            case BufferType::Mat3:
            case BufferType::Mat4:
                return GL_FLOAT;
            default:
                log_error(fmt::format("Missing type conversion for {}", type));
                return GL_FLOAT;
        }
    }

    inline int getDataTypeSize(BufferType t) const {
        switch (t) {
            case BufferType::None:
                return 0;
            case BufferType::Float:
                return sizeof(float);
            case BufferType::Float2:
                return 2 * sizeof(float);
            case BufferType::Float3:
                return 3 * sizeof(float);
            case BufferType::Float4:
                return 4 * sizeof(float);
            case BufferType::Mat3:
                return 3 * 3 * sizeof(float);
            case BufferType::Mat4:
                return 4 * 4 * sizeof(float);
            case BufferType::Bool:
                return sizeof(bool);
            default:
                log_error(fmt::format("Missing buffer type size for {}", t));
                return -1;
        }
    }

    int getCount() const {
        switch (type) {
            case BufferType::None:
                return 0;
            case BufferType::Bool:
            case BufferType::Float:
                return 1;
            case BufferType::Float2:
                return 2;
            case BufferType::Float3:
                return 3;
            case BufferType::Float4:
                return 4;
            case BufferType::Mat3:
                return 3 * 3;
            case BufferType::Mat4:
                return 4 * 4;
            default:
                log_error(fmt::format("Missing count for {}", type));
                return -1;
        }
    }
};
struct BufferLayout {
    std::vector<BufferElem> elements;

    std::vector<BufferElem>::iterator begin() { return elements.begin(); }
    std::vector<BufferElem>::iterator end() { return elements.end(); }

    int stride;

    BufferLayout() {}

    BufferLayout(const std::initializer_list<BufferElem>& elem)
        : elements(elem) {
        int offset = 0;
        stride = 0;
        for (auto& element : elements) {
            element.offset = offset;
            offset += element.size;
            stride += element.size;
        }
    }
};

struct VertexBuffer {
    BufferLayout layout;

    virtual ~VertexBuffer() {}
    virtual void bind() const = 0;
    virtual void unbind() const = 0;
    virtual void setLayout(const BufferLayout& l) = 0;
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
    virtual void setLayout(const BufferLayout& l) override { layout = l; }
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

