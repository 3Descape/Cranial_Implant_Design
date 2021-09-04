#include <mutex>
#include <vector>

#include "Viewport.hpp"
#include "Camera.hpp"
#include "Window.hpp"
#include "UISettings.hpp"

#ifndef APPLICATION_H
#define APPLICATION_H

class SceneObject;
class AreaPicker;
class NrrdResource;
class MeshResource;
class Skull;
typedef unsigned int GLuint;
typedef struct IntersectionPoint IntersectionPoint;
namespace boost { namespace filesystem { class path; }}

// TODO: only temporary
typedef enum Attribute_Index {
    ATTRIB_INDEX_POSITION = 0,
    ATTRIB_INDEX_COLOR = 1,
    ATTRIB_INDEX_NORMAL = 2,
    ATTRIB_INDEX_UV = 3,
} Attribute_Index;

#define ATTRIB_SIZE_POSITION 3
#define ATTRIB_SIZE_COLOR 4
#define ATTRIB_SIZE_UV 2
#define ATTRIB_SIZE_NORMAL 3

typedef struct Buffers {
    GLuint vao;
    GLuint ebo;
    uint64_t count;
    GLuint vbos[4];
} Buffers;

class Application
{
    private:
        const std::string name_ = "CranialImplant";

        GLuint fbo_; // TODO: refactor to its own framebuffer class
        GLuint color_texture_; // TODO: refactor to its own texture class

        uint32_t skull_count_;
        int createShaders();
        int createCoordinateAxis();
        int createAreaPicker();
        void updateDefaultTargetObjectIfNotSet();
        Viewport viewport_ = Viewport("Viewport");
        Window window_ = Window(name_);
        Camera camera_;
        float last_frame_ = 0.0f;

    public:
        std::mutex scene_objects_lock;
        std::vector<SceneObject*> scene_objects;
        int32_t selected_object_index = -1;
        int32_t selection_range_start = -1;
        int32_t selection_range_end = -1;
        int32_t target_index = -1;
        bool show_demo_window = false;
        bool update_cursor_position = false;
        bool hide_all_source_skulls = false;
        AreaPicker* area_picker = nullptr;
        ICPSettings icp_settings = {
            false, // use_same_normal_direction
            true, // use_symmetric_objective
            true, // use_reciprocal_correspondences
            false, // swap_source_and_target
            0.001f, // transformation_epsilon
            60, // max_iterations_count
            0.1f, // outside_weight
            1.0f, // inside_weight
            true, // use_weights
            0.5f, // normal_rejector_threshold
            nullptr, // area_picker
            false, // with_correspondence_vis
            false, // with_source_vis
            false, // with_target_vis
            true, // reset_transform
            ICPType::LLS_CLIPPED_AREAPICKER,
        };
        std::vector<IntersectionPoint> user_selection;
        float selection_radius = 2.0f;
        float angle_threshold = 0.3;
        float advection_scale = 1.0f;
        bool show_user_selection = true;
        bool show_region_selection = true;

        Application() = default;
        Application(const Application& application) = delete;
        ~Application();

        int init();
        void processInput();
        void update();
        void startUIFrame();
        void endUIFrame();
        void drawScene();
        void draw();
        void drawUI();
        bool shouldRun();
        void run();

        Window& getWindow();
        Camera& getCamera();

        Skull* getTargetObject();
        SceneObject* getSelectedObject();
        void addSceneObject(SceneObject* object);
        void deleteSceneObject(int64_t index);

        void addNrrdObject(const boost::filesystem::path& path);
        void addNrrdObject(const NrrdResource& nrrd_resource);
        void addNrrdObjects(const std::vector<boost::filesystem::path>& paths);
        void addMeshObjects(const std::vector<MeshResource>& mesh_resources);
        void addMeshObject(const MeshResource& mesh_resource);

        uint32_t getSkullCount();
        bool selectedObjectIsValidAlignSource() const;
        bool isValidAlignTarget() const;
        std::vector<SceneObject*> getVisibleAlignSources() const;
        std::vector<SceneObject*> getAlignSources() const;
};

extern Application app;

#endif