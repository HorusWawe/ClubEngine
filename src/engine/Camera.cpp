#include "Camera.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>
namespace club {
glm::mat4 Camera::view() const {
 glm::vec3 d{std::cos(glm::radians(yaw_))*std::cos(glm::radians(pitch_)),std::sin(glm::radians(pitch_)),std::sin(glm::radians(yaw_))*std::cos(glm::radians(pitch_))};
 return glm::lookAt(position_,position_+glm::normalize(d),glm::vec3(0,1,0));
}
glm::mat4 Camera::projection(float aspect,float fov,float nearP,float farP) const {
 glm::mat4 m=glm::perspective(glm::radians(fov),aspect,nearP,farP); m[1][1]*=-1.0f; return m;
}
}
