#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "util/util_opengl.hpp"

#ifndef CAMERA_H
#define CAMERA_H

class Camera
{
    private:
        float movement_speed_ = 20.0f;
        float delta_ = 1.0f;
        glm::vec3 position_ = glm::vec3(.0f, -60.f, 10.f);
        glm::vec3 direction_ = glm::vec3(.0f, -1.f, .0f);
        glm::vec3 up_ = glm::vec3(.0f, .0f, 1.f);
        float yaw_ = 90.f;
        float pitch_ = .0f;
        float foV_ = 45.f;
        float near_ = .1f;
        float far_ = 1000.f;

        float rotation_speed_ = 1.0f;
        float last_mouse_position_x_ = 0.0f;
        float last_mouse_position_y_ = 0.0f;
        bool first_mouse_ = true;

    public:
        glm::mat4 getLookAt() const { return glm::lookAt(xyzToxzy(position_), xyzToxzy(position_ - direction_), xyzToxzy(up_)); }
        glm::mat4 getProjection(float aspect_ratio) const { return glm::perspective(glm::radians(getFoV()), aspect_ratio, near_, far_); }
        float getFoV() const { return foV_; }
        float getNear() const { return near_; }
        float getFar() const { return far_; }
        void moveForward() { position_ -= (movement_speed_ * delta_) * direction_; }
        void moveBackward() { position_ += (movement_speed_ * delta_) * direction_; }
        void moveRight() { position_ -= glm::normalize(glm::cross(direction_, up_)) * (movement_speed_ * delta_); }
        void moveLeft() { position_ += glm::normalize(glm::cross(direction_, up_)) * (movement_speed_ * delta_); }
        void moveUp() { position_ += glm::vec3(.0f, .0f, 1.f) * (movement_speed_ * delta_); }
        void moveDown() { position_ -= glm::vec3(.0f, .0f, 1.f) * (movement_speed_ * delta_); }
        void setDelta(float delta) { delta_ = delta; }
        glm::vec3 getPosition() const { return position_; }
        glm::vec3 getU() const { return glm::normalize(glm::cross(up_, direction_)); }
        glm::vec3 getV() const { return glm::cross(direction_, getU()); }
        glm::vec3 getW() const { return direction_; }
        void setLastCursorPos(double mouse_x, double mouse_y) { last_mouse_position_x_ = mouse_x; last_mouse_position_y_ = mouse_y; }
        void getLastCursorPos(float& mouse_x, float& mouse_y) { mouse_x = last_mouse_position_x_; mouse_y = last_mouse_position_y_; }
        void updateRotation(double mouse_x, double mouse_y)
        {
            if (first_mouse_) // initially set to true
            {
                last_mouse_position_x_ = mouse_x;
                last_mouse_position_y_ = mouse_y;
                first_mouse_ = false;
            }

            float x_offset = mouse_x - last_mouse_position_x_;
            float y_offset = mouse_y - last_mouse_position_y_;

            last_mouse_position_x_ = mouse_x;
            last_mouse_position_y_ = mouse_y;

            yaw_ += x_offset * rotation_speed_ * delta_;
            pitch_ += y_offset * rotation_speed_ * delta_;

            if(pitch_ > 89.0f)
                pitch_ =  89.0f;
            if(pitch_ < -89.0f)
                pitch_ = -89.0f;

            glm::vec3 direction;
            direction.x = cos(glm::radians(yaw_)) * cos(glm::radians(pitch_));
            direction.y = sin(glm::radians(yaw_)) * cos(glm::radians(pitch_));
            direction.z = sin(glm::radians(pitch_));
            direction_ = -glm::normalize(direction);
        }
};

#endif // CAMERA_H
