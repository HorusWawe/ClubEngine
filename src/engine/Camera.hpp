#pragma once
#include <glm/glm.hpp>
namespace club {
class Camera {
public:
 void setPosition(const glm::vec3& p) { position_=p; }
 const glm::vec3& position() const { return position_; }
 glm::mat4 view() const;
 glm::mat4 projection(float aspect,float fovDegrees=70.0f,float nearPlane=0.1f,float farPlane=500.0f) const;
private:
 glm::vec3 position_{0.0f,1.7f,4.0f};
 float yaw_{-90.0f},pitch_{0.0f};
};
}
