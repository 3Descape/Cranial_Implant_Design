#include <vector>
#include <map>
#include <string>
#include <cstdint>

#include <glad/gl.h>

#include "SoA.hpp"
#include "Shader.hpp"

#ifndef OPENGL_OBJECT_H
#define OPENGL_OBJECT_H

typedef struct UniformValue
{
    void* data;
    uint32_t count = 1;
} UniformValue;

typedef struct UniformLayout
{
    std::string name;
    UniformType type;
    void* data;
    uint32_t count = 1;
} UniformLayout;

const std::vector<UniformLayout> UniformLayoutDefault = {
    {"model", UniformType::MAT4, nullptr},
    {"view", UniformType::MAT4, nullptr},
    {"projection", UniformType::MAT4, nullptr},
};

typedef struct AttributeLayout
{
    typedef enum Attribute
    {
        POSITION = 0,
        COLOR = 1,
        NORMAL = 2,
    } Attribute;

    std::string name;
    uint32_t attribute;
    uint32_t count;
    uint32_t data_type;
    uint32_t data_type_size;
    size_t offset;
    bool normalized = false;
} AttributeLayout;

const std::vector<AttributeLayout> LayoutPosition = {
    { "position", AttributeLayout::POSITION, 3, GL_FLOAT, sizeof(float), 0 },
};

const std::vector<AttributeLayout> LayoutColor = {
    { "position", AttributeLayout::POSITION, 3, GL_FLOAT, sizeof(float), 0 },
    { "color", AttributeLayout::COLOR, 3, GL_FLOAT, sizeof(float), 0 },
};

const std::vector<AttributeLayout> LayoutNormal = {
    { "position", AttributeLayout::POSITION, 3, GL_FLOAT, sizeof(float), 0 },
    { "normal", AttributeLayout::NORMAL, 3, GL_FLOAT, sizeof(float), 0 },
};

const std::vector<AttributeLayout> LayoutColorNormal = {
    { "position", AttributeLayout::POSITION, 3, GL_FLOAT, sizeof(float), 0 },
    { "color", AttributeLayout::COLOR, 4, GL_FLOAT, sizeof(float), 0 },
    { "normal", AttributeLayout::NORMAL, 3, GL_FLOAT, sizeof(float), 0 },
};

typedef struct OpenglBufferFlag
{
    typedef enum Flag
    {
        NONE = 0,
        VAO = 1 << 0,
        VBO = 1 << 1,
        EBO = 1 << 2,
        ALL = -1,
    } Flag;

    OpenglBufferFlag(uint32_t flag = Flag::NONE) : value(flag) {}

    uint32_t value;
 } OpenglBufferFlag;

bool operator&(const OpenglBufferFlag flag1, const OpenglBufferFlag::Flag& flag2);
bool operator&(const OpenglBufferFlag::Flag& flag1, const OpenglBufferFlag flag2);
OpenglBufferFlag operator|(const OpenglBufferFlag::Flag& flag1, const OpenglBufferFlag::Flag& flag2);

typedef struct OpenglObjectDescriptor
{
    ArrayLayout::Layout array_layout;
    void* data = nullptr;
    size_t data_size = 0;
    void* indices = nullptr;
    size_t indices_size = 0;
    size_t indices_count = 0;
    std::vector<AttributeLayout> attribute_layouts;
    Shader* shader = nullptr;
    OpenglBufferFlag buffer_flag = uint32_t(OpenglBufferFlag::ALL);
    SoA_Region_List regions;
} OpenglObjectDescriptor;

class OpenGLObject
{
    private:
        Shader* shader_ = nullptr;

    protected:
        GLuint vao_ = -1;
        GLuint vbo_ = -1;
        GLuint ebo_ = -1;
        uint32_t element_count_ = 0;
        std::vector<UniformLayout> uniform_layout_;
        OpenglBufferFlag buffer_flag_;

    public:
        OpenGLObject();
        OpenGLObject(const std::vector<UniformLayout>& uniform_layout, Shader* shader = nullptr);
        ~OpenGLObject();

        int bufferData(const OpenglObjectDescriptor& descriptor) const;
        void setVertexAttributeLayoutAoS(const std::vector<AttributeLayout>& layout) const;
        void setVertexAttributeLayoutSoA(const std::vector<AttributeLayout>& layout) const;
        void generateBuffers(OpenglBufferFlag buffer_flag = OpenglBufferFlag::ALL);
        void deleteBuffers();
        void setShader(Shader* shader);
        Shader* getShader() const;
        int addUniformLayout(const UniformLayout& layout);
        int addUniformLayout(const std::vector<UniformLayout>& layouts);
        int updateUniforms(const std::map<std::string, UniformValue>& additional_data) const;
        int updateUniforms(const std::vector<UniformLayout>& uniforms) const;
        GLuint getVAO() const;
        GLuint getVBO() const;
        GLuint getEBO() const;
        void bindVAO() const;
        void unbindVAO() const;
        void bindVBO() const;
        void unbindVBO() const;
        void bindEBO() const;
        void unbindEBO() const;
        void drawIndexed() const;
        void drawLine(GLfloat width = 1) const;
        void drawPoint(GLfloat size = 1) const;
        virtual void draw() const = 0;
};

#endif // OPENGL_OBJECT_H
