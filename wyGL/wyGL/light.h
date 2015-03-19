#pragma once
#include "linear_algebra.h"
#include "camera.h"
#include "mesh.h"
#include "material.h"

using namespace wyGL;
namespace wyGL
{
    class Light
    {
    public:
        Light();
        Light(const vec4d& _ambient, const vec4d& _diffuse, const vec4d& _specular, const vec4d& _position, 
              const vec3d _attenuation, const vec4d& _globalAmbient);
        void lighting( Mesh& _mesh, const Material& _material, const vec3d& _viewerPos);

        void setPosition(const vec4d& _position);
    private:
        // color of ambient light
        vec4d ambient_;
    
        // color of diffuse light
        vec4d diffuse_;

        // color of specular light
        vec4d specular_;

        // position of light
        vec4d position_;

        // attenuation factor
        vec3d attenuation_;

        // color of global ambient light
        vec4d globalAmbient_;
    };
}