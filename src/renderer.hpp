#pragma once

#include <glad/glad.h>

#include <string>
#include <thread>
#include "bsh_assert.hpp"
#include "logger/logger.hpp"

extern std::thread::id render_thread_id;

// TODO: move this to .cpp once all external calls are refactored to renderer interface calls
std::string opengl_transalte_error_code(GLuint code);
#ifdef DEBUG
#define OPENGL_CHECK(X) BSH_ASSERT_MSG(render_thread_id == std::this_thread::get_id(), "OpenGL command called from non render thread."); \
    X; { \
    GLenum err; \
    while((err = glGetError()) != GL_NO_ERROR) { \
    LOG_ERROR("OpenGL error: {}({:#x}), File: {}:{}", opengl_transalte_error_code(err), err, __FILE__, __LINE__); \
    }}
#else
#define OPENGL_CHECK(X) X
#endif

// size of a vector
#define R_SIZE(x) (x).size() * sizeof((x)[0])

#define R_STORAGE_COUNT_MAX 5

enum r_attribute {
    ATTRIBUTE_POSITION = 0,
    ATTRIBUTE_COLOR = 1,
    ATTRIBUTE_NORMAL = 2,
    ATTRIBUTE_UV = 3,
};

enum r_storage_index {
    STORAGE_INDEX_POSITION = 0,
    STORAGE_INDEX_COLOR = 1,
    STORAGE_INDEX_NORMAL = 2,
    STORAGE_INDEX_UV = 3,
    STORAGE_INDEX_EBO = 4,
};

enum r_attribute_size {
    ATTRIBUTE_SIZE_POSITION = 3,
    ATTRIBUTE_SIZE_COLOR = 4,
    ATTRIBUTE_SIZE_NORMAL = 3,
    ATTRIBUTE_SIZE_UV = 2,
};

struct r_attribute_layout {
    bool initalized = false;
    GLuint vao;
};

struct r_attribute_to_storage_buffer_link {
    GLuint attribute_index;
    GLuint storage_index;
    GLuint stride_bytes;
    GLuint buffer_start_offset_bytes;
};

#define R_LINKS_COUNT(x) sizeof(x) / sizeof(r_attribute_to_storage_buffer_link)

struct r_buffer {
    GLuint vbo = -1;
    GLsizei size_bytes = 0;
};

struct r_storage {
    bool initalized = false;
    GLuint element_count;
    GLuint buffers_count;
    r_buffer buffers[R_STORAGE_COUNT_MAX]; // TODO: make this dynamic by allocating dynamic array
};

struct r_framebuffer {
    bool initalized = false;
    GLuint fbo;
};

struct r_texture {
    bool initalized = false;
    GLuint tbo;
    GLuint width;
    GLuint height;
    GLenum type;
};

struct r_color {
    GLfloat r = 0.0f;
    GLfloat g = 0.0f;
    GLfloat b = 0.0f;
    GLfloat a = 1.0f;
};

int r_renderer_initalize();
void r_renderer_destroy();
void r_renderer_frame_begin();
void r_renderer_frame_end();

void r_attribute_layout_create(r_attribute_layout& layout);
void r_attribute_layout_set(r_attribute_layout& layout, GLuint attribute_index, GLuint element_count, GLenum type = GL_FLOAT);
void r_attribute_layout_set_multiple(r_attribute_layout& layout, GLuint attributes_count, GLuint attributes_indices[R_STORAGE_COUNT_MAX], GLuint attribute_element_counts[R_STORAGE_COUNT_MAX]);
void r_attribute_layout_activate(r_attribute_layout& layout);
void r_attribute_layout_destroy(r_attribute_layout& layout);

void r_storage_initalize(r_storage& storage);
void r_storage_buffer_create(r_storage& storage, GLuint buffer_index);
void r_storage_buffer_destroy(r_storage& storage, GLuint buffer_index);
void r_storage_buffers_create(r_storage& storage, GLuint buffer_indices_count, GLuint buffer_indices[R_STORAGE_COUNT_MAX]);
void r_storage_buffer_data_load(r_storage& storage, GLuint storage_index, GLsizeiptr size_bytes, const void* data, GLenum usage = GL_STATIC_DRAW);
void r_storage_buffer_data_load(r_storage& storage, GLuint storage_index, GLintptr offset, GLsizeiptr size_bytes, const void* data);
GLsizei r_storage_buffer_size_get(r_storage& storage, GLuint storage_index);
void r_storage_element_count_set(r_storage& storage, GLuint element_count);
void r_storage_buffer_set_binding_point(r_storage& storage, GLuint buffer_index, GLenum target, GLuint binding_point);
void r_link_attribute_layout_to_storage_buffer(r_attribute_layout& layout, r_storage& storage, GLuint attribute_index, GLuint storage_index, GLuint stride_bytes, GLuint buffer_start_offset_bytes);
void r_link_attribute_layout_to_storage_buffer(r_attribute_layout& layout, r_storage& storage, GLuint links_count, const r_attribute_to_storage_buffer_link links[R_STORAGE_COUNT_MAX]);
void r_storage_draw(r_storage& storage, GLenum draw_type, GLint start, GLsizei count);
void r_storage_draw(r_storage& storage, GLenum draw_type);
void r_storage_draw_indexed(r_storage& storage, GLenum draw_type = GL_TRIANGLES, GLuint ebo_storage_index = STORAGE_INDEX_EBO);
void r_storage_destroy(r_storage& storage);

void r_framebuffer_initalize(r_framebuffer& framebuffer);
void r_framebuffer_texture_attach(r_framebuffer& framebuffer, r_texture& texture, GLenum attachment, GLint texture_level = 0);
void r_framebuffer_enable(r_framebuffer& framebuffer, GLenum target = GL_FRAMEBUFFER);
void r_framebuffer_disable(r_framebuffer& framebuffer, GLenum target = GL_FRAMEBUFFER);
void r_framebuffer_clear_color(r_framebuffer& framebuffer, r_color color);
void r_framebuffer_clear_depth(r_framebuffer& framebuffer, GLfloat depth = 1.0f);
void r_framebuffer_destroy(r_framebuffer& framebuffer);

void r_viewport_size_set(int width, int height);

void r_texture_create(r_texture& texture, GLenum type, GLenum format, GLsizei width, GLsizei height, GLsizei texture_levels_count = 1);
void r_texture_parameter_set_int(r_texture& texture, GLenum parameter, GLint value);
void r_texture_parameter_set_int_array(r_texture& texture, GLenum parameter, const GLint* value);
void r_texture_parameter_set_float(r_texture& texture, GLenum parameter, GLfloat value);
void r_texture_parameter_set_float_array(r_texture& texture, GLenum parameter, const GLfloat* value);
void r_texture_data_load(r_texture& texture, GLsizei level, GLsizei width, GLsizei height, GLenum input_format, GLenum input_type, const void* data);
void r_texture_data_load(r_texture& texture, GLint level, GLsizei xoffset, GLsizei yoffset, GLsizei width, GLsizei height, GLenum input_format, GLenum input_type, const void* data);
void r_texture_mipmap_create(r_texture& texture);
void r_texture_bind(r_texture& texture, GLuint texture_unit = 0);
void r_texture_destroy(r_texture& texture);