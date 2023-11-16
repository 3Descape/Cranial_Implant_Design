#include<vector>
#include "SceneObject.hpp"
#include "opengl_object/Line.hpp"
#include "assert.hpp"

#ifndef SEQUENCE_LINE_OBJECT_H
#define SEQUENCE_LINE_OBJECT_H
class SequenceLineObject : public SceneObject
{
    protected:
        std::vector<Line*> lines_;
        float width_ = 1.0f;
        size_t sequence_index_ = 0;

        SceneObject* link_object_;

    public:
        SequenceLineObject(const std::string& name) : SceneObject(name, nullptr, Type::SEQUENCE_LINE_OBJECT) {}
        ~SequenceLineObject()
        {
            for(Line* line : lines_)
            {
                delete line;
            }
        }

        Line* addTimeStep(const OpenglObjectDescriptor& descriptor, const std::vector<UniformLayout>& uniform_layout)
        {
            lines_.emplace_back(new Line(descriptor, uniform_layout));
            object_ = lines_[lines_.size() - 1];

            return lines_[lines_.size() - 1];
        }

        void makeLink(SceneObject* object)
        {
            link_object_ = object;
        }

        void setSequenceIndex(size_t index)
        {
            ASSERT(lines_.size() > 0);

            if(index < 0) index = 0;
            if(index >= lines_.size()) index = lines_.size() - 1;

            sequence_index_ = index;
            object_ = getLine();

            link_object_->transformation_is_overriden = true;
            link_object_->transformation_override = transformations_[sequence_index_ + 1]; // +1 for default transform
        }

        size_t getSequenceIndex() const
        {
            return sequence_index_;
        }

        size_t getSize() const
        {
            return lines_.size();
        }

        Line* getLine()
        {
            ASSERT(lines_.size() > 0);
            return lines_[sequence_index_];
        }

        void setWidth(float width)
        {
            width_ = width;
        }

        float& getWidth()
        {
            return width_;
        }

        float getWidth() const
        {
            return width_;
        }
};
#endif