#pragma once

#include <string>
#include <fstream>
#include <sstream>
#include <glad/gl.h>
#include <glm/glm.hpp>

#include "logger/logger.hpp"
#include "util/util_opengl.hpp"

typedef enum class UniformType {
    INT,
    UINT,
    BOOL,
    FLOAT,
    VEC2,
    VEC3,
    VEC4,
    MAT4,
} UniformType;

class Shader
{
    public:
        typedef struct ShaderSources {
            const std::string name;
            const std::string vertex_shader;
            const std::string fragment_shader;
            const std::string geometry_shader;
        } ShaderSources;

    private:
        bool valid_ = false;
        const std::string name_;
        inline static std::map<std::string, Shader> shaders_;
        inline static std::string current_bound_shader_ = "";
        // utility function for checking shader compilation/linking errors.
        // returns != 0 on error
        // ------------------------------------------------------------------------
        int checkCompileErrors(GLuint shader, const std::string& type)
        {
            GLint success;
            GLchar infoLog[1024];
            if(type != "PROGRAM") {
                glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
                if(!success) {
                    glGetShaderInfoLog(shader, 1024, NULL, infoLog);
                    LOG_ERROR("SHADER_COMPILATION_ERROR of type: {}\n{}\nin Shader {}", type, infoLog, getName());
                    return -1;
                }
            }
            else {
                glGetProgramiv(shader, GL_LINK_STATUS, &success);
                if(!success) {
                    glGetProgramInfoLog(shader, 1024, NULL, infoLog);
                    LOG_ERROR("PROGRAM_LINKING_ERROR of type: {}\n{}\n in Shader {}", type, infoLog, getName());
                    return -2;
                }
            }

            return 0;
        }

    public:
        unsigned int ID = -1;
        // constructor generates the shader on the fly
        // ------------------------------------------------------------------------
        Shader(const std::string& name, const char* vertexPath, const char* fragmentPath, const char* geometryPath = nullptr) : name_(name)
        {
            // 1. retrieve the vertex/fragment source code from filePath
            std::string vertexCode;
            std::string fragmentCode;
            std::string geometryCode;
            std::ifstream vShaderFile;
            std::ifstream fShaderFile;
            std::ifstream gShaderFile;
            // ensure ifstream objects can throw exceptions:
            vShaderFile.exceptions (std::ifstream::failbit | std::ifstream::badbit);
            fShaderFile.exceptions (std::ifstream::failbit | std::ifstream::badbit);
            gShaderFile.exceptions (std::ifstream::failbit | std::ifstream::badbit);
            try {
                // open files
                vShaderFile.open(vertexPath);
                std::stringstream vShaderStream;
                // read file's buffer contents into streams
                vShaderStream << vShaderFile.rdbuf();
                // close file handlers
                vShaderFile.close();
                // convert stream into string
                vertexCode = vShaderStream.str();
            }
            catch (std::ifstream::failure& e) {
                LOG_ERROR("VERTEX_SHADER_FILE_NOT_SUCCESFULLY_READ: Shader: {}, Path: {} ", getName(), vertexPath);
                return;
            }

            try {
                // open files
                fShaderFile.open(fragmentPath);
                std::stringstream fShaderStream;
                // read file's buffer contents into streams
                fShaderStream << fShaderFile.rdbuf();
                // close file handlers
                fShaderFile.close();
                // convert stream into string
                fragmentCode = fShaderStream.str();
            }
            catch (std::ifstream::failure& e) {
                LOG_ERROR("FRAGMENT_SHADER_FILE_NOT_SUCCESFULLY_READ: Shader: {}, Path: {}", getName(), fragmentPath);
                return;
            }

            if (geometryPath != nullptr) {
                try {
                    gShaderFile.open(geometryPath);
                    std::stringstream gShaderStream;
                    gShaderStream << gShaderFile.rdbuf();
                    gShaderFile.close();
                    geometryCode = gShaderStream.str();
                }
                catch (std::ifstream::failure& e) {
                    LOG_ERROR("GEOMETRY_SHADER_FILE_NOT_SUCCESFULLY_READ: Shader: {}, Path: {}", getName(), geometryPath);
                    return;
                }
            }

            const char* vShaderCode = vertexCode.c_str();
            const char * fShaderCode = fragmentCode.c_str();
            // 2. compile shaders
            unsigned int vertex, fragment;
            // vertex shader
            vertex = glCreateShader(GL_VERTEX_SHADER);
            glShaderSource(vertex, 1, &vShaderCode, NULL);
            glCompileShader(vertex);
            if(checkCompileErrors(vertex, "VERTEX")) {
                LOG_ERROR("Failed compiling vertex shader {} (shader {})", vertexPath, getName());
            }
            // fragment Shader
            fragment = glCreateShader(GL_FRAGMENT_SHADER);
            glShaderSource(fragment, 1, &fShaderCode, NULL);
            glCompileShader(fragment);
            if(checkCompileErrors(fragment, "FRAGMENT")) {
                LOG_ERROR("Failed compiling fragment shader {} (shader {})", fragmentPath, getName());
            }
            // if geometry shader is given, compile geometry shader
            unsigned int geometry;
            if(geometryPath != nullptr) {
                const char * gShaderCode = geometryCode.c_str();
                geometry = glCreateShader(GL_GEOMETRY_SHADER);
                glShaderSource(geometry, 1, &gShaderCode, NULL);
                glCompileShader(geometry);
                if(checkCompileErrors(geometry, "GEOMETRY")) {
                    LOG_ERROR("Failed compiling geometry shader {} (shader {})", geometryPath, getName());
                }
            }
            // shader Program
            ID = glCreateProgram();
            glAttachShader(ID, vertex);
            glAttachShader(ID, fragment);
            if(geometryPath != nullptr) {
                glAttachShader(ID, geometry);
            }
            glLinkProgram(ID);
            if(checkCompileErrors(ID, "PROGRAM")) {
                LOG_ERROR("Failed linking shader {}", name);
            }
            // delete the shaders as they're linked into our program now and no longer necessery
            glDeleteShader(vertex);
            glDeleteShader(fragment);
            if(geometryPath != nullptr) {
                glDeleteShader(geometry);
            }

            valid_ = true;
        }

        Shader(const Shader& other) = default;
        Shader& operator=(const Shader& other) = default;

        static void clear()
        {
            for(auto pair : shaders_) {
                glDeleteProgram(pair.second.ID);
            }
        }

        bool isValid() const
        {
            return valid_;
        }

        std::string getName() const
        {
            return name_;
        }

        int save() const
        {
            if(shaders_.find(getName()) != shaders_.end()) {
                LOG_WARN("Shader {} already exists.", getName());
                return -1;
            }

            shaders_.insert(std::pair<std::string, Shader>(getName(), *this));

            return 0;
        }

        static bool exists(const std::string& name)
        {
            return shaders_.find(name) != shaders_.end();
        }

        static Shader* get(const std::string& name)
        {
            auto pair = shaders_.find(name);
            if(pair == shaders_.end()) {
                LOG_ERROR("Couldn't find shader {}", name);
                exit(-1);
            }

            return &pair->second;
        }

        // activate the shader
        // ------------------------------------------------------------------------
        Shader* use()
        {
            if(current_bound_shader_ == getName()) {
                return this;
            }

            current_bound_shader_ = getName();
            OPENGL_CHECK(glUseProgram(ID));

            return this;
        }

        static Shader* bindShader(const std::string& name)
        {
            Shader* shader = get(name);
            if(!shader) {
                return 0;
            }

            shader->use();

            return shader;
        }

        GLint getUniformLocation(const std::string& name) const
        {
            return getUniformLocation(name.c_str());
        }

        GLint getUniformLocation(const char* name) const
        {
            GLint location = OPENGL_CHECK(glGetUniformLocation(ID, name));
            if(location == -1) {
                LOG_ERROR("Uniform location {} could not be found in shader {}", name, getName());
                return -1;
            }
            return location;
        }

        // utility uniform functions
        // ------------------------------------------------------------------------
        void setBool(const std::string& name, bool value) const
        {
            OPENGL_CHECK(glUniform1i(getUniformLocation(name), (int)value));
        }
        // ------------------------------------------------------------------------
        void setInt(const std::string& name, int32_t value) const
        {
            OPENGL_CHECK(glUniform1i(getUniformLocation(name), value));
        }
        void setUInt(const std::string& name, uint32_t value) const
        {
            OPENGL_CHECK(glUniform1ui(getUniformLocation(name), value));
        }
        // ------------------------------------------------------------------------
        void setFloat(const std::string& name, float value) const
        {
            OPENGL_CHECK(glUniform1f(getUniformLocation(name), value));
        }
        // ------------------------------------------------------------------------
        void setVec2(const std::string& name, const glm::vec2& value, uint32_t count = 1) const
        {
            OPENGL_CHECK(glUniform2fv(getUniformLocation(name), count, &value[0]));
        }
        void setVec2(const std::string& name, float x, float y) const
        {
            OPENGL_CHECK(glUniform2f(getUniformLocation(name), x, y));
        }
        // ------------------------------------------------------------------------
        void setVec3(const std::string& name, const glm::vec3& value, uint32_t count = 1) const
        {
            OPENGL_CHECK(glUniform3fv(getUniformLocation(name), count, &value[0]));
        }
        void setVec3(const std::string& name, float x, float y, float z) const
        {
            OPENGL_CHECK(glUniform3f(getUniformLocation(name), x, y, z));
        }
        // ------------------------------------------------------------------------
        void setVec4(const std::string& name, const glm::vec4& value, uint32_t count = 1) const
        {
            OPENGL_CHECK(glUniform4fv(getUniformLocation(name), count, &value[0]));
        }
        void setVec4(const std::string& name, float x, float y, float z, float w)
        {
            OPENGL_CHECK(glUniform4f(getUniformLocation(name), x, y, z, w));
        }
        // ------------------------------------------------------------------------
        void setMat2(const std::string& name, const glm::mat2& mat, uint32_t count = 1) const
        {
            OPENGL_CHECK(glUniformMatrix2fv(getUniformLocation(name), count, GL_FALSE, &mat[0][0]));
        }
        // ------------------------------------------------------------------------
        void setMat3(const std::string& name, const glm::mat3& mat, uint32_t count = 1) const
        {
            OPENGL_CHECK(glUniformMatrix3fv(getUniformLocation(name), count, GL_FALSE, &mat[0][0]));
        }
        // ------------------------------------------------------------------------
        void setMat4(const std::string& name, const glm::mat4& mat, uint32_t count = 1) const
        {
            OPENGL_CHECK(glUniformMatrix4fv(getUniformLocation(name), count, GL_FALSE, &mat[0][0]));
        }
        // ------------------------------------------------------------------------
        int setUniform(const std::string& uniform_name, UniformType type, void* data, uint32_t count = 1) const
        {
            if(data == nullptr) {
                LOG_ERROR("Shader::setUniform(): data for uniform {} was nullptr (shader {}).", uniform_name, getName());
                return -1;
            }

            if(count == 0) {
                LOG_ERROR("Shader::setUniform(): count for uniform {} was 0 (shader {}).", uniform_name, getName());
                return -1;
            }

            switch (type)
            {
            case UniformType::BOOL:
                setBool(uniform_name, *(bool*)data);
                break;
            case UniformType::INT:
                setInt(uniform_name, *(int32_t*)data);
                break;
            case UniformType::UINT:
                setUInt(uniform_name, *(uint32_t*)data);
                break;
            case UniformType::FLOAT:
                setFloat(uniform_name, *(float*)data);
                break;
            case UniformType::VEC2:
                setVec2(uniform_name, *(glm::vec2*)data, count);
                break;
            case UniformType::VEC3:
                setVec3(uniform_name, *(glm::vec3*)data, count);
                break;
            case UniformType::VEC4:
                setVec4(uniform_name, *(glm::vec4*)data, count);
                break;
            case UniformType::MAT4:
                setMat4(uniform_name, *(glm::mat4*)data, count);
                break;
            default:
                LOG_ERROR("Shader::setUniform(): UniformType {}(for entry {}) not supported(shader {})", static_cast<int>(type), uniform_name, getName());
                return -1;
                break;
            }

            return 0;
        }
};