
#include "Camera.hpp"
#include <SDL3/SDL_scancode.h>
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>
#include <cmath>
namespace club {
glm::mat4 Camera::view()const{glm::vec3 d{cos(glm::radians(yaw_))*cos(glm::radians(pitch_)),sin(glm::radians(pitch_)),sin(glm::radians(yaw_))*cos(glm::radians(pitch_))};return glm::lookAt(position_,position_+glm::normalize(d),{0,1,0});}
glm::mat4 Camera::projection(float a,float f,float n,float z)const{auto m=glm::perspective(glm::radians(f),a,n,z);m[1][1]*=-1;return m;}
void Camera::rotate(float x,float y){yaw_+=x*.12f;pitch_=std::clamp(pitch_-y*.12f,-85.0f,85.0f);}
void Camera::move(const bool*k,float dt){glm::vec3 f{cos(glm::radians(yaw_)),0,sin(glm::radians(yaw_))};glm::vec3 r=glm::normalize(glm::cross(f,{0,1,0}));float s=3.5f*dt;if(k[SDL_SCANCODE_W])position_+=f*s;if(k[SDL_SCANCODE_S])position_-=f*s;if(k[SDL_SCANCODE_A])position_-=r*s;if(k[SDL_SCANCODE_D])position_+=r*s;if(k[SDL_SCANCODE_SPACE])position_.y+=s;if(k[SDL_SCANCODE_LCTRL])position_.y-=s;}
}
