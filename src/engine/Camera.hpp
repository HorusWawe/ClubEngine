
#pragma once
#include <glm/glm.hpp>
namespace club {
class Camera {
public:
 glm::mat4 view() const; glm::mat4 projection(float aspect,float fovDegrees=70,float nearPlane=.1f,float farPlane=500)const;
 void rotate(float dx,float dy); void move(const bool* keys,float dt);
private: glm::vec3 position_{0,1.65f,4.2f};float yaw_{-90},pitch_{-8};
};
}
