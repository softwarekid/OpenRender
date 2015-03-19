#include "stdafx.h"
#include "light.h"
#include <math.h>
Light::Light(const vec4d& _ambient, const vec4d& _diffuse, const vec4d& _specular, const vec4d& _position, 
             const vec3d _attenuation,const vec4d& _globalAmbient )
             :ambient_(_ambient), 
             diffuse_(_diffuse),
             specular_(_specular),
             position_(_position),
             attenuation_(_attenuation),
             globalAmbient_(_globalAmbient)
{

}

Light::Light()
{
    ambient_ = vec4d(0.5, 0.5, 0.5, 1.0);
    diffuse_ = vec4d(1.0, 1.0, 1.0, 1.0);
    specular_ = vec4d(1.0, 1.0, 1.0, 1.0);
    attenuation_ = vec3d(1.0, 0.0, 0.0);
    globalAmbient_ = vec4d(0.2, 0.2, 0.2, 1.0);
}

void Light::lighting(Mesh& _mesh, const Material& _material, const vec3d& _viewerPos)
{
    for (int i = 0; i < _mesh.pointsSize(); i++)
    {
        vec4d finalColor(0.0,0.0,0.0,1.0);

        Point3D& p = _mesh.point(i);
        vec4d homoCoord = p.coordinate_;
        if (homoCoord[3] < 0.00001)
        {
            homoCoord[3] = 0.00001;
        }
        vec3d vertexPos = vec3d(homoCoord[0] / homoCoord[3], homoCoord[1] / homoCoord[3], homoCoord[2] / homoCoord[3]);
        vec3d lightPos;
        if (fabs(position_[3]) > 0.00001)
        {
            lightPos = vec3d(position_[0] / position_[3], position_[1] / position_[3], position_[2] / position_[3]);
        }
        else
        {
            lightPos = vec3d(position_[0],position_[1],position_[2]);
        }

        // emission color from material
        finalColor = finalColor + _material.emission_;

        // global ambient color
        finalColor = finalColor + (globalAmbient_ * _material.ambient_);

        // diffuse light page 459
        vec3d vertexToLight = lightPos - vertexPos; 
        // for directional light, L is the opposite direction of light
        if((fabs(position_[3]) <= 0.00001))
        {
            vertexToLight = vec3d(-lightPos[0],-lightPos[1],-lightPos[2]);
        }
        vertexToLight.normalize();
        // cos value of L and N
        double fCos = vertexToLight.dotProduct(p.normal_);
        fCos = (fCos < 0.0f) ? 0.0f : fCos;

        finalColor = finalColor + (diffuse_ * _material.diffuse_ * fCos);

        // specular light
        // page 462
        if (fCos > 0.0)
        {
            // use halfway vector to approximate Phong model
            vec3d vertexToViewer = _viewerPos - vertexPos;
            vertexToViewer.normalize();
            vertexToViewer = vertexToViewer + vertexToLight;
            vertexToViewer.normalize();
            double fShine = vertexToViewer.dotProduct(p.normal_);
            fShine = (fShine < 0.0f) ? 0.0f : fShine;
            double fShineFactor = (float)pow(fShine,  _material.shininess_);
            finalColor = finalColor + specular_ * _material.specular_ * fShineFactor;
        }

        // clamp color to [0,1]
        finalColor[0] = (finalColor[0] < 0.0) ? 0.0 : ((finalColor[0] > 1.0)? 1.0 : finalColor[0]);
        finalColor[1] = (finalColor[1] < 0.0) ? 0.0 : ((finalColor[1] > 1.0)? 1.0 : finalColor[1]);
        finalColor[2] = (finalColor[2] < 0.0) ? 0.0 : ((finalColor[2] > 1.0)? 1.0 : finalColor[2]);

        // transform color to [0,255]
        p.color_ = vec3d(finalColor[0],finalColor[1],finalColor[2]);
    }
}

void wyGL::Light::setPosition(const vec4d& _position)
{
    position_ = _position;
}




