#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "OpenGLObject.hpp"
#include "util/util_opengl.hpp"
#include "logger/logger.hpp"
#include "Shader.hpp"


bool operator&(const OpenglBufferFlag flag1, const OpenglBufferFlag::Flag& flag2)
{
    return uint32_t(flag1.value) & uint32_t(flag2);
}

bool operator&(const OpenglBufferFlag::Flag& flag1, const OpenglBufferFlag flag2)
{
  return uint32_t(flag1) & uint32_t(flag2.value);
}

OpenglBufferFlag operator|(const OpenglBufferFlag::Flag& flag1, const OpenglBufferFlag::Flag& flag2)
{
  return { uint32_t(flag1) | uint32_t(flag2) };
}

OpenGLObject::OpenGLObject()
{
};

OpenGLObject::OpenGLObject(const std::vector<UniformLayout>& uniform_layout, Shader* shader) : shader_(shader)
{
    addUniformLayout(uniform_layout);
}

OpenGLObject::~OpenGLObject()
{
    deleteBuffers();
}

int OpenGLObject::bufferData(const OpenglObjectDescriptor& descriptor) const
{
    if(buffer_flag_ & OpenglBufferFlag::VAO) bindVAO();
    if(buffer_flag_ & OpenglBufferFlag::VBO)
    {
        bindVBO();
        OPENGL_CHECK(glBufferData(GL_ARRAY_BUFFER, descriptor.data_size, descriptor.data, GL_STATIC_DRAW));
    }

    if(buffer_flag_ & OpenglBufferFlag::EBO)
    {
        bindEBO();
        OPENGL_CHECK(glBufferData(GL_ELEMENT_ARRAY_BUFFER, descriptor.indices_size, descriptor.indices, GL_STATIC_DRAW));
    }

    if(descriptor.array_layout == ArrayLayout::SoA)
        setVertexAttributeLayoutSoA(descriptor.attribute_layouts);
    else
        setVertexAttributeLayoutAoS(descriptor.attribute_layouts);

    if(buffer_flag_ & OpenglBufferFlag::VAO) unbindVAO();
    if(buffer_flag_ & OpenglBufferFlag::VBO) unbindVBO();
    if(buffer_flag_ & OpenglBufferFlag::EBO) unbindEBO();

    return 0;
}

void OpenGLObject::setVertexAttributeLayoutAoS(const std::vector<AttributeLayout>& layout) const
{
    uint32_t stride = 0;
    size_t offset = 0;
    for(auto entry : layout) stride += entry.count * entry.data_type_size;
    for(auto entry : layout)
    {
        if(
        entry.data_type == GL_BYTE ||
        entry.data_type == GL_UNSIGNED_BYTE ||
        entry.data_type == GL_SHORT ||
        entry.data_type == GL_UNSIGNED_SHORT ||
        entry.data_type == GL_INT ||
        entry.data_type == GL_UNSIGNED_INT)
        {
            OPENGL_CHECK(glVertexAttribIPointer(entry.attribute, entry.count, entry.data_type, stride, (void*)offset));
            LOG_INFO("Integer Attribute: {}", entry.name);
        }
        else {
            OPENGL_CHECK(glVertexAttribPointer(entry.attribute, entry.count, entry.data_type, entry.normalized, stride, (void*)offset));
        }

        OPENGL_CHECK(glEnableVertexAttribArray(entry.attribute));
        offset += entry.count * entry.data_type_size;
    }
}

void OpenGLObject::setVertexAttributeLayoutSoA(const std::vector<AttributeLayout>& layout) const
{
    for(auto entry : layout)
    {
        if(
        entry.data_type == GL_BYTE ||
        entry.data_type == GL_UNSIGNED_BYTE ||
        entry.data_type == GL_SHORT ||
        entry.data_type == GL_UNSIGNED_SHORT ||
        entry.data_type == GL_INT ||
        entry.data_type == GL_UNSIGNED_INT)
        {
            OPENGL_CHECK(glVertexAttribIPointer(entry.attribute, entry.count, entry.data_type, entry.count * entry.data_type_size, (void*)entry.offset));
            LOG_INFO("Integer Attribute: {}", entry.name);
        }
        else
            OPENGL_CHECK(glVertexAttribPointer(entry.attribute, entry.count, entry.data_type, entry.normalized, entry.count * entry.data_type_size, (void*)entry.offset));

        OPENGL_CHECK(glEnableVertexAttribArray(entry.attribute));
    }
}

void OpenGLObject::generateBuffers(OpenglBufferFlag buffer_flag)
{
    if(buffer_flag & OpenglBufferFlag::VAO) OPENGL_CHECK(glGenVertexArrays(1, &vao_));
    if(buffer_flag & OpenglBufferFlag::VBO) OPENGL_CHECK(glGenBuffers(1, &vbo_));
    if(buffer_flag & OpenglBufferFlag::EBO) OPENGL_CHECK(glGenBuffers(1, &ebo_));

    buffer_flag_ = buffer_flag;
}

void OpenGLObject::deleteBuffers()
{
    if(buffer_flag_ & OpenglBufferFlag::VAO) OPENGL_CHECK(glDeleteVertexArrays(1, &vao_));
    if(buffer_flag_ & OpenglBufferFlag::VBO) OPENGL_CHECK(glDeleteBuffers(1, &vbo_));
    if(buffer_flag_ & OpenglBufferFlag::EBO) OPENGL_CHECK(glDeleteBuffers(1, &ebo_));

    vao_ = -1;
    vbo_ = -1;
    ebo_ = -1;
}

void OpenGLObject::setShader(Shader* shader)
{
    shader_ = shader;
}

Shader* OpenGLObject::getShader() const
{
    return shader_;
}

int OpenGLObject::addUniformLayout(const UniformLayout& layout)
{
    for(auto& l : uniform_layout_)
        if(l.name == layout.name) return -1;

    uniform_layout_.push_back(layout);

    return 0;
}

int OpenGLObject::addUniformLayout(const std::vector<UniformLayout>& layouts)
{
    for (const auto& l : layouts)
    {
        if(int code = addUniformLayout(l))
        return code;
    }

    return 0;
}

int OpenGLObject::updateUniforms(const std::map<std::string, UniformValue>& additional_data) const
{
    for(auto& entry : uniform_layout_)
    {
        uint32_t count = 1;
        void* data = entry.data;
        if(data == nullptr)
        {
            auto val = additional_data.find(entry.name);
            if(val == additional_data.end()) {
                LOG_ERROR("OpenGLObject::updateUniforms(): Tried to set value for {} but data == nullptr and no additional data was provided!", entry.name);
                continue;
            }

            data = val->second.data;
            count = val->second.count;
        }

        shader_->setUniform(entry.name, entry.type, data, count);
    }

    return 0;
}

int OpenGLObject::updateUniforms(const std::vector<UniformLayout>& uniforms) const
{
    for(const UniformLayout& entry : uniforms)
        shader_->setUniform(entry.name, entry.type, entry.data, entry.count);

    return 0;
}

GLuint OpenGLObject::getVAO() const
{
    return vao_;
}

GLuint OpenGLObject::getVBO() const
{
    return vbo_;
}

GLuint OpenGLObject::getEBO() const
{
    return ebo_;
}

void OpenGLObject::bindVAO() const
{
    OPENGL_CHECK(glBindVertexArray(vao_));
}

void OpenGLObject::unbindVAO() const
{
    OPENGL_CHECK(glBindVertexArray(0));
}

void OpenGLObject::bindVBO() const
{
    OPENGL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, vbo_));
}

void OpenGLObject::unbindVBO() const
{
    OPENGL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, 0));
}

void OpenGLObject::bindEBO() const
{
    OPENGL_CHECK(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_));
}

void OpenGLObject::unbindEBO() const
{
    OPENGL_CHECK(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
}

void OpenGLObject::drawIndexed() const
{
    bindVAO();
    bindEBO();
    OPENGL_CHECK(glDrawElements(GL_TRIANGLES, element_count_, GL_UNSIGNED_INT, 0));
    unbindEBO();
    unbindVAO();
}

void OpenGLObject::drawLine(GLfloat width) const
{
    bindVAO();
    bindEBO();
    OPENGL_CHECK(glEnable(GL_LINE_SMOOTH));
    GLfloat old_line_width;
    OPENGL_CHECK(glGetFloatv(GL_LINE_WIDTH, &old_line_width));
    OPENGL_CHECK(glLineWidth(width));
    OPENGL_CHECK(glDrawElements(GL_LINES, element_count_, GL_UNSIGNED_INT, 0));
    OPENGL_CHECK(glLineWidth(old_line_width));
    OPENGL_CHECK(glDisable(GL_LINE_SMOOTH));
    unbindEBO();
    unbindVAO();
}

void OpenGLObject::drawPoint(GLfloat size) const
{
    bindVAO();
    GLfloat old_point_size;
    OPENGL_CHECK(glGetFloatv(GL_POINT_SIZE, &old_point_size));
    OPENGL_CHECK(glPointSize(size));
    OPENGL_CHECK(glDrawArrays(GL_POINTS, 0, element_count_));
    OPENGL_CHECK(glPointSize(old_point_size));
    OPENGL_CHECK(glDisable(GL_LINE_SMOOTH));
    unbindVAO();
}