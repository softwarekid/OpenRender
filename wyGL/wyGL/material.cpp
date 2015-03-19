#include "stdafx.h"
#include "material.h"

namespace wyGL
{
    Material::Material()
    {     
        ambient_ = vec4d(0.5, 0.5, 0.5, 1.0);
        diffuse_ = vec4d(0.8, 0.8, 0.8, 1.0);
        specular_ = vec4d(0.2, 0.2, 0.2, 1.0);
        emission_ = vec4d(0.0, 0.0, 0.0, 1.0);
        shininess_ = 128.0;
    }

}