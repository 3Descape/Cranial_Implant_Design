// This software is in the public domain. Where that dedication is not
// recognized, you are granted a perpetual, irrevocable license to copy,
// distribute, and modify this file as you see fit.
// https://github.com/ddiakopoulos/tinyply

// This file is only required for the example and test programs.

#pragma once

#ifndef tinyply_example_utils_hpp
#define tinyply_example_utils_hpp

#include <thread>
#include <vector>
#include <sstream>
#include <fstream>
#include <iostream>
#include <cstring>
#include <iterator>
#include <openvdb/openvdb.h>
#include <glm/glm.hpp>

#include "tinyply.h"
#include "util/util_timer.hpp"

using namespace tinyply;

inline std::vector<uint8_t> read_file_binary(const std::string & pathToFile)
{
    std::ifstream file(pathToFile, std::ios::binary);
    std::vector<uint8_t> fileBufferBytes;

    if (file.is_open())
    {
        file.seekg(0, std::ios::end);
        size_t sizeBytes = file.tellg();
        file.seekg(0, std::ios::beg);
        fileBufferBytes.resize(sizeBytes);
        if (file.read((char*)fileBufferBytes.data(), sizeBytes)) return fileBufferBytes;
    }
    else throw std::runtime_error("could not open binary ifstream to path " + pathToFile);
    return fileBufferBytes;
}

struct memory_buffer : public std::streambuf
{
    char * p_start {nullptr};
    char * p_end {nullptr};
    size_t size;

    memory_buffer(char const * first_elem, size_t size)
        : p_start(const_cast<char*>(first_elem)), p_end(p_start + size), size(size)
    {
        setg(p_start, p_start, p_end);
    }

    pos_type seekoff(off_type off, std::ios_base::seekdir dir, std::ios_base::openmode which) override
    {
        if (dir == std::ios_base::cur) gbump(static_cast<int>(off));
        else setg(p_start, (dir == std::ios_base::beg ? p_start : p_end) + off, p_end);
        return gptr() - p_start;
    }

    pos_type seekpos(pos_type pos, std::ios_base::openmode which) override
    {
        return seekoff(pos, std::ios_base::beg, which);
    }
};

struct memory_stream : virtual memory_buffer, public std::istream
{
    memory_stream(char const * first_elem, size_t size)
        : memory_buffer(first_elem, size), std::istream(static_cast<std::streambuf*>(this)) {}
};

inline void write_ply(const std::string& filename, const std::vector<glm::uvec3>& triangles, const std::vector<glm::vec3>& vertices)
{
    std::filebuf fb_binary;
    fb_binary.open(filename, std::ios::out | std::ios::binary);
    std::ostream outstream_binary(&fb_binary);
    if (outstream_binary.fail()) throw std::runtime_error("failed to open " + filename);

    PlyFile ply_file;

    ply_file.add_properties_to_element("vertex", { "x", "y", "z" },
        Type::FLOAT32, vertices.size(), reinterpret_cast<const uint8_t*>(vertices.data()), Type::INVALID, 0);

    ply_file.add_properties_to_element("face", { "vertex_indices" },
        Type::UINT32, triangles.size(), reinterpret_cast<const uint8_t*>(triangles.data()), Type::UINT8, 3);

    // Write a binary file
    ply_file.write(outstream_binary, true);
}

inline int read_ply(const std::string& filepath, std::vector<glm::vec3>* points_out, std::vector<glm::uvec3>* triangles_out, const bool preload_into_memory = true)
{
    std::cout << "..................TINYPLY.................................\n";
    std::cout << "Now Reading: " << filepath << std::endl;

    std::unique_ptr<std::istream> file_stream;
    std::vector<uint8_t> byte_buffer;

    try
    {
        // For most files < 1gb, pre-loading the entire file upfront and wrapping it into a
        // stream is a net win for parsing speed, about 40% faster.
        if(preload_into_memory)
        {
            byte_buffer = read_file_binary(filepath);
            file_stream.reset(new memory_stream((char*)byte_buffer.data(), byte_buffer.size()));
        }
        else
            file_stream.reset(new std::ifstream(filepath, std::ios::binary));

        if (!file_stream || file_stream->fail())
            throw std::runtime_error("file_stream failed to open " + filepath);

        file_stream->seekg(0, std::ios::end);
        const float size_mb = file_stream->tellg() * float(1e-6);
        file_stream->seekg(0, std::ios::beg);

        PlyFile file;
        file.parse_header(*file_stream);

        std::shared_ptr<PlyData> vertices, triangles;

        vertices = file.request_properties_from_element("vertex", { "x", "y", "z" });
        triangles = file.request_properties_from_element("face", { "vertex_indices" }, 3);

        Timer read_timer;
        file.read(*file_stream);
        read_timer.stop();

        std::cout << "\tparsing " << size_mb << "mb in " << read_timer.toString() << " [" << (size_mb / read_timer.seconds()) << " MBps]" << std::endl;

        if(vertices)
        {
            std::cout << "\tRead " << vertices->count  << " total vertices"<< std::endl;
            points_out->resize(vertices->count);
            std::memcpy(points_out->data(), vertices->buffer.get(), vertices->buffer.size_bytes());
        }

        if(triangles)
        {
            std::cout << "\tRead " << triangles->count     << " total triangles" << std::endl;
            triangles_out->resize(triangles->count);
            std::memcpy(triangles_out->data(), triangles->buffer.get(), triangles->buffer.size_bytes());
        }
    }
    catch (const std::exception & e)
    {
        std::cerr << "Caught tinyply exception: " << e.what() << std::endl;
        return -1;
    }

    return 0;
}

inline void read_blender_ply(const std::string& filepath, std::vector<glm::vec3>& points_out, std::vector<glm::uvec3>& indices_out, std::vector<glm::vec4>& colors_out, std::vector<glm::vec3>& normals_out, const bool preload_into_memory = true)
{
    std::cout << "..................TINYPLY.................................\n";
    std::cout << "Now Reading: " << filepath << std::endl;

    std::unique_ptr<std::istream> file_stream;
    std::vector<uint8_t> byte_buffer;

    try
    {
        // For most files < 1gb, pre-loading the entire file upfront and wrapping it into a
        // stream is a net win for parsing speed, about 40% faster.
        if (preload_into_memory)
        {
            byte_buffer = read_file_binary(filepath);
            file_stream.reset(new memory_stream((char*)byte_buffer.data(), byte_buffer.size()));
        }
        else
        {
            file_stream.reset(new std::ifstream(filepath, std::ios::binary));
        }

        if (!file_stream || file_stream->fail()) throw std::runtime_error("file_stream failed to open " + filepath);

        file_stream->seekg(0, std::ios::end);
        const float size_mb = file_stream->tellg() * float(1e-6);
        file_stream->seekg(0, std::ios::beg);

        PlyFile file;
        file.parse_header(*file_stream);

        // Because most people have their own mesh types, tinyply treats parsed data as structured/typed byte buffers.
        // See examples below on how to marry your own application-specific data structures with this one.
        std::shared_ptr<PlyData> vertices, normals, colors, faces;

        try { vertices = file.request_properties_from_element("vertex", { "x", "y", "z" }); }
        catch (const std::exception & e) { std::cerr << "tinyply exception: " << e.what() << std::endl; }

        try { faces = file.request_properties_from_element("face", { "vertex_indices" }, 3); }
        catch (const std::exception & e) { std::cerr << "tinyply exception: " << e.what() << std::endl; }

        try { normals = file.request_properties_from_element("vertex", { "nx", "ny", "nz" }); }
        catch (const std::exception & e) { std::cerr << "tinyply exception: " << e.what() << std::endl; }

        try { colors = file.request_properties_from_element("vertex", { "red", "green", "blue", "alpha"}); }
        catch (const std::exception & e) { std::cerr << "tinyply exception: " << e.what() << std::endl; }

        file.read(*file_stream);

        assert(vertices->t == tinyply::Type::FLOAT32);
        assert(normals->t == tinyply::Type::FLOAT32);
        assert(colors->t == tinyply::Type::UINT8);

        const size_t numFacesBytes = faces->buffer.size_bytes();
        indices_out.resize(faces->count);
        std::memcpy(indices_out.data(), faces->buffer.get(), numFacesBytes);

        const size_t numVerticesBytes = vertices->buffer.size_bytes();
        points_out.resize(vertices->count);
        std::memcpy(points_out.data(), vertices->buffer.get(), numVerticesBytes);

        colors_out.reserve(colors->count);
        const uint8_t* color_data = (const uint8_t*)colors->buffer.get();
        for(int i = 0; i < colors->count; ++i)
        {
            glm::vec4 color(color_data[0], color_data[1], color_data[2], color_data[3]);
            color_data += 4;
            colors_out.push_back(color / 255.f);
        }

        const size_t numNormalsBytes = normals->buffer.size_bytes();
        normals_out.resize(normals->count);
        std::memcpy(normals_out.data(), normals->buffer.get(), numNormalsBytes);
    }
    catch (const std::exception & e)
    {
        std::cerr << "Caught tinyply exception: " << e.what() << std::endl;
    }
}

#endif
