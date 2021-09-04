#include <vector>

#ifndef SoA_H
#define SoA_H

typedef struct ArrayLayout
{
    enum Layout {
        SoA,
        AoS,
    };
} ArrayLayout;

typedef struct SoA_Region
{
    const char* name;
    const size_t element_size;
    const size_t element_count;
    size_t offset = 0;
} SoA_Region;

typedef struct SoA_Memcpy_Descriptor
{
    typedef enum SourceType {
        ARRAY,
        CONSTANT,
    } Type;

    SoA_Region region;
    SourceType src_value_type;
    void* src;
} SoA_Memcpy_Descriptor;

typedef struct SoA_Region_List
{
    const std::vector<SoA_Region> regions;

    const SoA_Region* get(const char* name);
} SoA_Region_List;

SoA_Region_List SoA_create_region_list(const std::vector<SoA_Memcpy_Descriptor>& descriptors);
size_t SoA_get_size_bytes(const std::vector<SoA_Memcpy_Descriptor>& descriptors);
void SoA_memcpy(void* destination, const std::vector<SoA_Memcpy_Descriptor>& descriptors);

#endif //SoA_H