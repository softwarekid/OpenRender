#pragma once
#include "linear_algebra.h"

namespace wyGL
{
    struct Material
    {
        Material(const vec4d& _ambient, const vec4d& _diffuse, const vec4d& _specular, const vec4d& _emission, const float _shininess):
            ambient_(_ambient), diffuse_(_diffuse), specular_(_specular), emission_(_emission), shininess_(_shininess)
        {

        }

        Material();
        vec4d ambient_;
        vec4d diffuse_;
        vec4d specular_;
        vec4d emission_;
        double shininess_;
    };

}