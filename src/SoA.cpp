#include <cstring>
#include <iostream>

#include "SoA.hpp"

const SoA_Region* SoA_Region_List::get(const char* name)
{
    for(auto& region : regions)
    {
        if(!std::strcmp(region.name, name))
            return &region;
    }

    std::cout << "Error: SoA_Region_List failed to find name \"" << name << "\"" << std::endl;

    return nullptr;
}

SoA_Region_List SoA_create_region_list(const std::vector<SoA_Memcpy_Descriptor>& descriptors)
{
    std::vector<SoA_Region> regions;

    size_t offset = 0;
    for(auto& descriptor : descriptors)
    {
        SoA_Region temp_region = descriptor.region;
        temp_region.offset = offset;
        regions.push_back(temp_region);

        offset += descriptor.region.element_count * descriptor.region.element_size;
    }

    return { regions };
}

size_t SoA_get_size_bytes(const std::vector<SoA_Memcpy_Descriptor>& descriptors)
{
    size_t size = 0;
    for(auto& descriptor : descriptors)
        size += descriptor.region.element_size * descriptor.region.element_count;
    return size;
}

void SoA_memcpy(void* destination, const std::vector<SoA_Memcpy_Descriptor>& descriptors)
{
    char* dest = (char*)destination;
    for(auto& descriptor : descriptors)
    {
        if(descriptor.src == nullptr) continue;

        switch (descriptor.src_value_type)
        {
            case SoA_Memcpy_Descriptor::CONSTANT: {
                for(int i = 0; i < descriptor.region.element_count; ++i)
                {
                    std::memcpy(dest, descriptor.src, descriptor.region.element_size);
                    dest += descriptor.region.element_size;
                }
            } break;
            case SoA_Memcpy_Descriptor::ARRAY: {
                std::memcpy(dest, descriptor.src, descriptor.region.element_size * descriptor.region.element_count);
                dest += descriptor.region.element_size * descriptor.region.element_count;
            } break;
            default:
            {
                std::cout << "SoA_Memcpy_Descrptor SrcType not implemented" << std::endl;
                exit(-1);
                break;
            }
        }
    }
}