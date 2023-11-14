class AreaPicker;

#ifndef UI_H
#define UI_H

typedef struct ICPType
{
    typedef enum Type {
        LLS = 0,
        LLS_CLIPPED_SELECTION = 1,
        LLS_CLIPPED_AREAPICKER = 2,
    } Type;

    Type value = LLS;

    ICPType(const Type& type) : value(type) {};

    inline static Type elements[] = { LLS, LLS_CLIPPED_SELECTION, LLS_CLIPPED_AREAPICKER };
    inline static const char* labels[] = { "None", "User Selection + Region Picker", "Region Picker" };

    ICPType& operator=(const ICPType& other)
    {
        value = other.value;
        return *this;
    }

    ICPType& operator=(const Type& other)
    {
        value = other;
        return *this;
    }

    bool operator==(const ICPType& rhs) const
    {
        return value == rhs.value;
    }

    bool operator==(const Type& rhs) const
    {
        return value == rhs;
    }

    bool operator!=(const Type& rhs) const
    {
        return value != rhs;
    }

    static const char* getLabel(const Type type)
    {
        return labels[type];
    }

    const char* getLabel()
    {
        return labels[value];
    }
} ICPType;

typedef struct ICPSettings
{
    bool use_same_normal_direction;
    bool use_symmetric_objective;
    bool use_reciprocal_correspondences;
    bool swap_source_and_target;
    float transformation_epsilon;
    int max_iterations_count;
    float outside_weight;
    float inside_weight;
    bool use_weights;
    float normal_rejector_threshold;
    const AreaPicker* area_picker;
    bool with_correspondence_vis;
    bool with_source_vis;
    bool with_target_vis;
    bool reset_transform;
    ICPType icp_type;
} ICPSettings;

#endif // UI_H