#include "renderer.hpp"

#include "util/opengl.hpp"
#include "logger/logger.hpp"
#include "shader.hpp"
#include "shader_definitions.hpp"

std::thread::id render_thread_id;

std::string opengl_transalte_error_code(GLuint code) {
    switch (code)
    {
    case GL_INVALID_ENUM:
        return "GL_INVALID_ENUM";
    case GL_INVALID_VALUE:
        return "GL_INVALID_VALUE";
    case GL_INVALID_OPERATION:
        return "GL_INVALID_OPERATION";
    case GL_STACK_OVERFLOW:
        return "GL_STACK_OVERFLOW";
    case GL_STACK_UNDERFLOW:
        return "GL_STACK_UNDERFLOW";
    case GL_OUT_OF_MEMORY:
        return "GL_OUT_OF_MEMORY";
    case GL_INVALID_FRAMEBUFFER_OPERATION:
        return "GL_INVALID_FRAMEBUFFER_OPERATION";
    case GL_CONTEXT_LOST:
        return "GL_CONTEXT_LOST";
    default:
        return "UNKNOWN";
    }
}

int r_renderer_initalize() {
    render_thread_id = std::this_thread::get_id();

    for(const auto& sources : shader_sources) {
        if(Shader::createShaderProgram(sources)) {
            return -1;
        }
    }

    return 0;
}

void r_renderer_destroy() {
    Shader::deleteShaders();
}

void r_attribute_layout_create(r_attribute_layout& layout) {
    OPENGL_CHECK(glCreateVertexArrays(1, &layout.vao));
    layout.initalized = true;
}

// TODO: support non floating point types via glVertexArrayAttrib*Format functions
void r_attribute_layout_set(r_attribute_layout& layout, GLuint attribute_index, GLuint element_count, GLenum type) {
    BSH_ASSERT_MSG_DEBUG(layout.initalized, "The attribute layout must be initalized first.");
    OPENGL_CHECK(glEnableVertexArrayAttrib(layout.vao, attribute_index));
    OPENGL_CHECK(glVertexArrayAttribFormat(layout.vao, attribute_index, element_count, type, false, 0));
}

void r_attribute_layout_set_multiple(r_attribute_layout& layout, GLuint attributes_count, GLuint attributes_indices[R_STORAGE_COUNT_MAX], GLuint attribute_element_counts[R_STORAGE_COUNT_MAX]) {
    BSH_ASSERT_MSG_DEBUG(layout.initalized, "The attribute layout must be initalized first.");
    for(int i = 0; i < attributes_count; ++i) {
        r_attribute_layout_set(layout, attributes_indices[i], attribute_element_counts[i], GL_FLOAT);
    }
}

void r_attribute_layout_activate(r_attribute_layout& layout) {
    BSH_ASSERT_MSG_DEBUG(layout.initalized, "The attribute layout must be initalized first.");
    OPENGL_CHECK(glBindVertexArray(layout.vao));
}

void r_attribute_layout_destroy(r_attribute_layout& layout) {
    OPENGL_CHECK(glDeleteVertexArrays(1, &layout.vao));
    layout.initalized = true;
}

void r_storage_initalize(r_storage& storage) {
    storage.element_count = -1;
    storage.buffers_count = R_STORAGE_COUNT_MAX;
    for(int i = 0; i < storage.buffers_count; ++i) {
        storage.buffers[i].vbo = -1;
        storage.buffers[i].size_bytes = 0;
    }
    storage.initalized = true;
}

void r_storage_buffer_create(r_storage& storage, GLuint buffer_index) {
    BSH_ASSERT_MSG_DEBUG(storage.initalized, "The storage must be initalized first.");
    BSH_ASSERT_MSG_DEBUG(0 <= buffer_index && buffer_index < storage.buffers_count, "The index was set to an invalid value.");
    BSH_ASSERT_MSG_DEBUG(storage.buffers[buffer_index].vbo == -1, "The buffer location is not empty.");
    OPENGL_CHECK(glCreateBuffers(1, &storage.buffers[buffer_index].vbo));
}

void r_storage_buffer_destroy(r_storage& storage, GLuint buffer_index) {
    BSH_ASSERT_MSG_DEBUG(storage.initalized, "The storage must be initalized first.");
    BSH_ASSERT_MSG_DEBUG(0 <= buffer_index && buffer_index < storage.buffers_count, "The index was set to an invalid value.");
    BSH_ASSERT_MSG_DEBUG(storage.buffers[buffer_index].vbo != -1, "The buffer location is not initalized.");
    OPENGL_CHECK(glDeleteBuffers(1, &storage.buffers[buffer_index].vbo));
    storage.buffers[buffer_index].vbo = -1;
    storage.buffers[buffer_index].size_bytes = 0;
}

void r_storage_buffers_create(r_storage& storage, GLuint buffer_indices_count, GLuint buffer_indices[R_STORAGE_COUNT_MAX]) {
    for(int i = 0; i < buffer_indices_count; ++i) {
        r_storage_buffer_create(storage, buffer_indices[i]);
    }
}

void r_storage_buffer_data_load(r_storage& storage, GLuint storage_index, GLsizeiptr size_bytes, const void* data, GLenum usage) {
    BSH_ASSERT_MSG_DEBUG(storage.initalized, "The storage must be initalized first.");
    BSH_ASSERT_MSG_DEBUG(storage.buffers[storage_index].vbo != -1, "The buffer object has not been initalized yet.");
    OPENGL_CHECK(glNamedBufferData(storage.buffers[storage_index].vbo, size_bytes, data, usage));
    storage.buffers[storage_index].size_bytes = size_bytes;
}

void r_storage_buffer_data_load(r_storage& storage, GLuint storage_index, GLintptr offset, GLsizeiptr size_bytes, const void* data) {
    BSH_ASSERT_MSG_DEBUG(storage.initalized, "The storage must be initalized first.");
    BSH_ASSERT_MSG_DEBUG(storage.buffers[storage_index].vbo != -1, "The buffer object has not been initalized yet.");
    BSH_ASSERT_MSG_DEBUG(0 <= offset && offset + size_bytes <= storage.buffers[storage_index].size_bytes, "Data range must lay inside currently allocated area.");
    OPENGL_CHECK(glNamedBufferSubData(storage.buffers[storage_index].vbo, offset, size_bytes, data));
}

GLsizei r_storage_buffer_size_get(r_storage& storage, GLuint storage_index) {
    BSH_ASSERT_MSG_DEBUG(storage.initalized, "The storage must be initalized first.");
    BSH_ASSERT_MSG_DEBUG(storage.buffers[storage_index].vbo != -1, "The buffer object has not been initalized yet.");
    return storage.buffers[storage_index].size_bytes;
}

void r_storage_element_count_set(r_storage& storage, GLuint element_count) {
    BSH_ASSERT_MSG_DEBUG(storage.initalized, "The storage must be initalized first.");
    storage.element_count = element_count;
}

/*
https://registry.khronos.org/OpenGL-Refpages/gl4/html/glBindBufferBase.xhtml
*/
void r_storage_buffer_set_binding_point(r_storage& storage, GLuint buffer_index, GLenum target, GLuint binding_point) {
    BSH_ASSERT_MSG_DEBUG(storage.initalized, "The storage must be initalized first.");
    BSH_ASSERT_MSG_DEBUG(storage.buffers[buffer_index].vbo != -1, "The buffer must be initalized first.");
    BSH_ASSERT_DEBUG(target == GL_ATOMIC_COUNTER_BUFFER || target ==  GL_TRANSFORM_FEEDBACK_BUFFER ||
                    target == GL_UNIFORM_BUFFER || target ==  GL_SHADER_STORAGE_BUFFER);
    BSH_ASSERT_DEBUG(binding_point < GL_MAX_SHADER_STORAGE_BUFFER_BINDINGS);
    glBindBufferBase(target, binding_point, storage.buffers[buffer_index].vbo);
}

void r_link_attribute_layout_to_storage_buffer(r_attribute_layout& layout, r_storage& storage, GLuint attribute_index, GLuint storage_index, GLuint stride_bytes, GLuint buffer_start_offset_bytes) {
    BSH_ASSERT_MSG_DEBUG(layout.initalized, "The layout must be initalized first.");
    BSH_ASSERT_MSG_DEBUG(storage.initalized, "The storage must be initalized first.");
    OPENGL_CHECK(glVertexArrayVertexBuffer(layout.vao, attribute_index, storage.buffers[storage_index].vbo, buffer_start_offset_bytes, stride_bytes));
}

void r_link_attribute_layout_to_storage_buffer(r_attribute_layout& layout, r_storage& storage, GLuint links_count, const r_attribute_to_storage_buffer_link links[R_STORAGE_COUNT_MAX]) {
    BSH_ASSERT_MSG_DEBUG(layout.initalized, "The layout must be initalized first.");
    BSH_ASSERT_MSG_DEBUG(storage.initalized, "The storage must be initalized first.");
    for(int i = 0; i < links_count; ++i) {
        r_link_attribute_layout_to_storage_buffer(layout, storage, links[i].attribute_index, links[i].storage_index, links[i].stride_bytes, links[i].buffer_start_offset_bytes);
    }
}

void r_storage_draw(r_storage& storage, GLenum draw_type, GLint start, GLsizei count) {
    BSH_ASSERT_MSG_DEBUG(storage.initalized, "The storage must be initalized first.");
    OPENGL_CHECK(glDrawArrays(draw_type, start, count));
}

void r_storage_draw(r_storage& storage, GLenum draw_type) {
    BSH_ASSERT_MSG_DEBUG(storage.element_count != -1, "The element count has not been set yet with r_storage_set_element_count().");
    r_storage_draw(storage, draw_type, 0, storage.element_count);
}

void r_storage_draw_indexed(r_storage& storage, GLenum draw_type, GLuint ebo_storage_index) {
    BSH_ASSERT_MSG_DEBUG(storage.initalized, "The storage must be initalized first.");
    BSH_ASSERT_MSG_DEBUG(storage.element_count != -1, "The element count has not been set yet with r_storage_set_element_count().");
    OPENGL_CHECK(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, storage.buffers[ebo_storage_index].vbo));
    OPENGL_CHECK(glDrawElements(draw_type, storage.element_count, GL_UNSIGNED_INT, nullptr));
}

void r_storage_destroy(r_storage& storage) {
    storage.element_count = -1;
    for(int i = 0; i < storage.buffers_count; ++i) {
        if(storage.buffers[i].vbo != -1)
            r_storage_buffer_destroy(storage, i);
    }
    storage.buffers_count = -1;
    storage.initalized = false;
}

void r_framebuffer_initalize(r_framebuffer& framebuffer) {
    OPENGL_CHECK(glCreateFramebuffers(1, &framebuffer.fbo));
    framebuffer.initalized = true;
}

void r_framebuffer_texture_attach(r_framebuffer& framebuffer, r_texture& texture, GLenum attachment, GLint texture_level) {
    BSH_ASSERT_MSG_DEBUG(framebuffer.initalized, "The framebuffer must first be initalized.");
    BSH_ASSERT_MSG_DEBUG(texture.initalized, "The texture must first be initalized.");
    OPENGL_CHECK(glNamedFramebufferTexture(framebuffer.fbo, attachment, texture.tbo, texture_level));
    BSH_ASSERT_MSG(glCheckNamedFramebufferStatus(framebuffer.fbo, GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE, "Failed attaching texture to framebuffer.");
}

void r_framebuffer_enable(r_framebuffer& framebuffer, GLenum target) {
    BSH_ASSERT_MSG_DEBUG(framebuffer.initalized, "The framebuffer must first be initalized.");
    OPENGL_CHECK(glBindFramebuffer(target, framebuffer.fbo));
}

void r_framebuffer_disable(r_framebuffer& framebuffer, GLenum target) {
    BSH_ASSERT_MSG_DEBUG(framebuffer.initalized, "The framebuffer must first be initalized.");
    OPENGL_CHECK(glBindFramebuffer(target, 0));
}

void r_framebuffer_clear_color(r_framebuffer& framebuffer, r_color color) {
    BSH_ASSERT_MSG_DEBUG(framebuffer.initalized, "The framebuffer must first be initalized.");
    // TODO: the drawbuffer is currently hardcoded to 0. make this more flexible
    // resource: https://stackoverflow.com/questions/50310483/glclearbufferfv-doesnt-clear-specified-color-attachment
    OPENGL_CHECK(glClearNamedFramebufferfv(framebuffer.fbo, GL_COLOR, 0, (GLfloat*)&color));
}

void r_framebuffer_clear_depth(r_framebuffer& framebuffer, GLfloat depth) {
    BSH_ASSERT_MSG_DEBUG(framebuffer.initalized, "The framebuffer must first be initalized.");
    OPENGL_CHECK(glClearNamedFramebufferfv(framebuffer.fbo, GL_DEPTH, 0, &depth));
}

void r_framebuffer_destroy(r_framebuffer& framebuffer) {
    BSH_ASSERT_MSG_DEBUG(framebuffer.initalized, "The framebuffer must first be initalized.");
    OPENGL_CHECK(glDeleteFramebuffers(1, &framebuffer.fbo));
    framebuffer.fbo = -1;
    framebuffer.initalized = false;
}

void r_viewport_size_set(int width, int height) {
    OPENGL_CHECK(glViewport(0, 0, width, height));
}

void r_texture_create(r_texture& texture, GLenum type, GLenum format, GLsizei width, GLsizei height, GLsizei texture_levels_count) {
    OPENGL_CHECK(glCreateTextures(type, 1, &texture.tbo));
    OPENGL_CHECK(glTextureStorage2D(texture.tbo, texture_levels_count, format, width, height)); // TODO: we might to perform this action individually
    texture.type = type;
    texture.width = width;
    texture.height = height;
    texture.initalized = true;
}

void r_texture_parameter_set_int(r_texture& texture, GLenum parameter, GLint value) {
    BSH_ASSERT_MSG_DEBUG(texture.initalized, "The texture must first be initalized.");
    OPENGL_CHECK(glTextureParameteri(texture.tbo, parameter, value));
}

void r_texture_parameter_set_int_array(r_texture& texture, GLenum parameter, const GLint* value) {
    BSH_ASSERT_MSG_DEBUG(texture.initalized, "The texture must first be initalized.");
    OPENGL_CHECK(glTextureParameteriv(texture.tbo, parameter, value));
}

void r_texture_parameter_set_float(r_texture& texture, GLenum parameter, GLfloat value) {
    BSH_ASSERT_MSG_DEBUG(texture.initalized, "The texture must first be initalized.");
    OPENGL_CHECK(glTextureParameterf(texture.tbo, parameter, value));
}

void r_texture_parameter_set_float_array(r_texture& texture, GLenum parameter, const GLfloat* value) {
    BSH_ASSERT_MSG_DEBUG(texture.initalized, "The texture must first be initalized.");
    OPENGL_CHECK(glTextureParameterfv(texture.tbo, parameter, value));
}

void r_texture_data_load(r_texture& texture, GLint level, GLsizei xoffset, GLsizei yoffset, GLsizei width, GLsizei height, GLenum input_format, GLenum input_type, const void* data) {
    BSH_ASSERT_MSG_DEBUG(texture.initalized, "The texture must first be initalized.");
    OPENGL_CHECK(glTextureSubImage2D(texture.tbo, level, xoffset, yoffset, width, height, input_format, input_type, data));
}

void r_texture_data_load(r_texture& texture, GLsizei level, GLsizei width, GLsizei height, GLenum input_format, GLenum input_type, const void* data) {
    r_texture_data_load(texture, level, 0, 0, width, height, input_format, input_type, data);
}

void r_texture_mipmap_create(r_texture& texture) {
    BSH_ASSERT_MSG_DEBUG(texture.initalized, "The texture must first be initalized.");
    OPENGL_CHECK(glGenerateTextureMipmap(texture.tbo));
}

void r_texture_bind(r_texture& texture, GLuint texture_unit) {
    BSH_ASSERT_MSG_DEBUG(texture.initalized, "The texture must first be initalized.");
    OPENGL_CHECK(glActiveTexture(GL_TEXTURE0));
    OPENGL_CHECK(glBindTextureUnit(texture_unit, texture.tbo));
}

void r_texture_destroy(r_texture& texture) {
    BSH_ASSERT_MSG_DEBUG(texture.initalized, "The texture must first be initalized.");
    OPENGL_CHECK(glDeleteTextures(1, &texture.tbo));
    texture.tbo = -1;
    texture.type = GL_INVALID_ENUM;
    texture.width = -1;
    texture.height = -1;
    texture.initalized = false;
}