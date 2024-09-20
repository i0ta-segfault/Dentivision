#include "glm/glm/gtc/type_ptr.hpp"
#include "glm/glm.hpp"
#include "glm/glm/gtc/matrix_transform.hpp"

class Camera {
public:
    glm::vec3 cameraPosition, cameraFront, cameraRight, cameraUp, worldUp;
    float yaw, pitch;
    float screenWidth, screenHeight;
    float fov;
    float znear, zfar;
    float sensitivity;

    enum class Movement {
        FORWARD,
        BACKWARD,
        LEFT,
        RIGHT,
        UP,
        DOWN
    };

    Camera(glm::vec3 camPosition, glm::vec3 camFront, glm::vec3 camUp, float fv, float scrWidth, float scrHeight, float zn, float zf) {
        cameraPosition = camPosition;
        cameraFront = camFront;
        worldUp = camUp;
        fov = fv;
        screenWidth = scrWidth;
        screenHeight = scrHeight;
        znear = zn;
        zfar = zf;
        yaw = -90.0f; // Default to looking along the negative z-axis
        pitch = 0.0f;
        sensitivity = 0.1f;
        updateCameraVectors();
    }

    glm::mat4 look(){
        return glm::lookAt(cameraPosition, cameraPosition + cameraFront, cameraUp);
    }

    glm::mat4 project(){
        return glm::perspective(glm::radians(fov), screenWidth / screenHeight, znear, zfar);
    }

    void move(Movement direction, float deltaTime) {
        float cameraSpeed = 2.5f * deltaTime;
        switch (direction) {
            case Movement::FORWARD:
                cameraPosition += cameraSpeed * cameraFront;
                break;
            case Movement::BACKWARD:
                cameraPosition -= cameraSpeed * cameraFront;
                break;
            case Movement::LEFT:
                cameraPosition -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
                break;
            case Movement::RIGHT:
                cameraPosition += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
                break;
            case Movement::UP:
                cameraPosition.y += cameraSpeed;
                break;
            case Movement::DOWN:
                cameraPosition.y -= cameraSpeed;
                break;
        }
    }

    void rotate(float xOffset, float yOffset, bool constrainPitch = true) {
        xOffset *= sensitivity;
        yOffset *= sensitivity;

        yaw += xOffset;
        pitch += yOffset;

        // to avoid gimbal lock
        if (constrainPitch) {
            if (pitch > 89.0f)
                pitch = 89.0f;
            if (pitch < -89.0f)
                pitch = -89.0f;
        }

        updateCameraVectors();
    }

    private:
    void updateCameraVectors() {
        glm::vec3 front;
        front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        front.y = sin(glm::radians(pitch));
        front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
        cameraFront = glm::normalize(front);
        cameraRight = glm::normalize(glm::cross(cameraFront, worldUp));
        cameraUp = glm::normalize(glm::cross(cameraRight, cameraFront));
    }

};
