#include "stdafx.h"
#include "mesh.h"
#include <assert.h>
#include <iostream>
#include <cstring>
#include <string>
#include <fstream>
#include <sstream>
#include <limits.h>

#define MINNUMBER -9999999

namespace wyGL
{
    int Mesh::rotateDegree = 0;
    Mesh::Mesh(const vector<vec4d>& _coords,const vector<vec2d>& _uvCoordinate, const vector<vec3i>& _indices)
    {
        points_.reserve(_coords.size());
        for (int i = 0; i < _coords.size(); i++)
        {
            Point3D p;
            p.coordinate_ = _coords[i];
            p.texture_ = _uvCoordinate[i];
            points_.push_back(p);
        }

        
        indices_ = _indices;
        faceNormalComputed = false;

        facesCulled.resize(_indices.size());
        for (int i = 0; i < _indices.size(); i++)
        {
            facesCulled[i] = 0;
        }
    }

    Mesh::Mesh()
    {
        faceNormalComputed = false;
    }

    Point3D& wyGL::Mesh::point(int i) 
    {
        return points_[i];
    }

    const Point3D& wyGL::Mesh::point(int i) const
    {
        return points_[i];
    }

    void Mesh::push_back(const Point3D& _p)
    {
        points_.push_back(_p);
    }

    void Mesh::reserve(unsigned int _size)
    {
        points_.reserve(_size);
    }

    const int Mesh::pointsSize() const 
    {
        return points_.size();
    }

    void Mesh::calculateFaceNormal()
    {
        faceNormals.clear();
        faceNormals.reserve(indices_.size());
        for (int i = 0; i < indices_.size(); i++)
        {
            vec4d homoCoordV0 = points_[indices_[i][0]].coordinate_;
            vec3d coordV0 = vec3d(homoCoordV0[0] / homoCoordV0[3], homoCoordV0[1] / homoCoordV0[3], homoCoordV0[2] / homoCoordV0[3]);

            vec4d homoCoordV1 = points_[indices_[i][1]].coordinate_;
            vec3d coordV1 = vec3d(homoCoordV1[0] / homoCoordV1[3], homoCoordV1[1] / homoCoordV1[3], homoCoordV1[2] / homoCoordV1[3]);

            vec4d homoCoordV2 = points_[indices_[i][2]].coordinate_;
            vec3d coordV2 = vec3d(homoCoordV2[0] / homoCoordV2[3], homoCoordV2[1] / homoCoordV2[3], homoCoordV2[2] / homoCoordV2[3]);

            vec3d v0Tov1 = coordV1 - coordV0;
            vec3d v0Tov2 = coordV2 - coordV0;

            vec3d faceNormal;
            faceNormal = v0Tov1.crossProduct(v0Tov2);
            faceNormal.normalize();

            faceNormals.push_back(faceNormal);
        }
        faceNormalComputed = true;
    }

    void Mesh::calculateVertexNormal()
    {
        if (!faceNormalComputed)
        {
            calculateFaceNormal();
        }

        for (int i = 0; i < points_.size(); i++)
        {
            vec3d vertexNormal;
            for (int j = 0; j < indices_.size(); j++)
            {
                if (i == indices_[j][0] || i == indices_[j][1] || i == indices_[j][2])
                {
                    vertexNormal = vertexNormal + faceNormals[j];
                }
            }
            vertexNormal.normalize();
            points_[i].normal_ = vertexNormal;
        }
    }

    vector<vec3i>& Mesh::getIndices()
    {
        return indices_;
    }

    const vector<vec3i>& Mesh::getIndices() const
    {
        return indices_;
    }

    vector<Point3D>& Mesh::getPoints()
    {
        return points_;
    }

    void Mesh::rotateY(const double _degree)
    {
        matrix4d rotateTrans;
        double cs = cos(_degree / 180.0 * PI);
        double sn = sin(_degree / 180.0 * PI);

        rotateTrans(0, 0) = cs;
        rotateTrans(0, 2) = sn;
        rotateTrans(1, 1) = 1;
        rotateTrans(2, 0) = -sn;
        rotateTrans(2, 2) = cs;
        rotateTrans(3, 3) = 1;

        for (int i = 0; i < points_.size(); i++)
        {
            points_[i].coordinate_ = points_[i].coordinate_.leftMatrixMultiply(rotateTrans);
        }

        for (int i = 0; i < faceNormals.size(); i++)
        {
            vec4d HomoNormal = vec4d(faceNormals[i][0], faceNormals[i][1], faceNormals[i][2],0);
            HomoNormal = HomoNormal.leftMatrixMultiply(rotateTrans);
            faceNormals[i] = vec3d(HomoNormal[0], HomoNormal[1], HomoNormal[2]);
        }

        for (int i = 0; i < points_.size(); i++)
        {
            vec4d HomoNormal = vec4d(points_[i].normal_[0], points_[i].normal_[1], points_[i].normal_[2],0);
            HomoNormal = HomoNormal.leftMatrixMultiply(rotateTrans);
            points_[i].normal_ = vec3d(HomoNormal[0], HomoNormal[1], HomoNormal[2]);
        }

        //calculateFaceNormal();
        rotateDegree += 10;
    }


    void Mesh::translate( const vec3d& _dis )
    {
        matrix4d translateMatrix;
        translateMatrix(0, 0) = 1;
        translateMatrix(1, 1) = 1;
        translateMatrix(2, 2) = 1;
        translateMatrix(3, 3) = 1;

        translateMatrix(0, 3) = _dis[0];
        translateMatrix(1, 3) = _dis[1];
        translateMatrix(2, 3) = _dis[2];

        for (int i = 0; i < points_.size(); i++)
        {
            points_[i].coordinate_ = points_[i].coordinate_.leftMatrixMultiply(translateMatrix);
        }

    }

    void Mesh::backCulling(const vec3d _cameraPosition)
    {
        if (!faceNormalComputed)
        {
            calculateFaceNormal();
        }

        for (int i = 0; i < indices_.size(); i++)
        {
            vec4d HomoP0 = points_[indices_[i][0]].coordinate_;
            vec4d HomoP1 = points_[indices_[i][1]].coordinate_;
            vec4d HomoP2 = points_[indices_[i][2]].coordinate_;
            vec3d p0;
            vec3d p1;
            vec3d p2;
            if ( fabs(HomoP0[3] - 1) < 0.00001 )
            {
                p0 = vec3d(HomoP0[0], HomoP0[1], HomoP0[2]);
                p1 = vec3d(HomoP1[0], HomoP1[1], HomoP1[2]);
                p2 = vec3d(HomoP2[0], HomoP2[1], HomoP2[2]);
            }
            else
            {
                p0 = vec3d( HomoP0[0] / HomoP0[3], HomoP0[1] / HomoP0[3], HomoP0[2] / HomoP0[3]);
                p1 = vec3d( HomoP1[0] / HomoP1[3], HomoP1[1] / HomoP1[3], HomoP1[2] / HomoP1[3]);
                p2 = vec3d( HomoP2[0] / HomoP2[3], HomoP2[1] / HomoP2[3], HomoP2[2] / HomoP2[3]);
            }
            if ((p0 - _cameraPosition).dotProduct(faceNormals[i]) >= 0 &&
                (p1 - _cameraPosition).dotProduct(faceNormals[i]) >= 0 &&
                (p2 - _cameraPosition).dotProduct(faceNormals[i]) >= 0 )
            {
                facesCulled[i] = 1;
            }
            //cout<<i;
        }
    }

    bool Mesh::isCulled(unsigned int i)
    {
        assert(i >= 0);
        return facesCulled[i];
    }

    bool Mesh::loadMesh(string _filename)
    {
        std::cout << "Loading mesh from '" << _filename << "'..." << std::endl;

        std::string line;
        std::ifstream file(_filename);

        if ( !file.is_open() ) 
        {
            std::cout << "Error opening file '" << _filename << "' for mesh loading.\n";
            return false;
        }

        int vertexNum = 0;
        int faceNum = 0;
        std::string token;

        while ( getline( file, line ) )
        {
            std::stringstream stream( line );
            stream >> token;

            if ( token == "v" ) 
            {
                vertexNum++;
            }
            else if (token == "f")
            {
                faceNum++;
            }
            token.clear();
        }

        points_.resize(vertexNum);
        indices_.resize(faceNum);
        facesCulled.resize(faceNum);

        file.clear();
        file.seekg(0,ios::beg);
        vertexNum = 0;
        faceNum = 0;
        double maxCoord = MINNUMBER;
        // two passes to get the 
        while ( getline( file, line ) )
        {
            std::stringstream stream( line );
            stream >> token;

            if ( token == "v" ) 
            {
                vec4d &coord = points_[vertexNum].coordinate_;
                stream >> coord[0] >> coord[1] >> coord[2];
                
                for (int i = 0; i < 3; i++)
                {
                    if (maxCoord < coord[i])
                    {
                        maxCoord = coord[i];
                    }
                }

                coord[3] = 1;
                vertexNum++;
            }
            else if (token == "f")
            {
                vec3i &index = indices_[faceNum];
                stream >> index[0] >> index[1] >> index[2];
                // index starts from 0
                index[0]--;
                index[1]--;
                index[2]--;
                faceNum++;
            }
            token.clear();
        }

        // unify the mesh coordinate
    
        for (int i = 0; i < points_.size(); i++)
        {
            points_[i].coordinate_ = points_[i].coordinate_ / maxCoord;
            points_[i].coordinate_[3] = 1;
        }

    }
}