#pragma once
#include "linear_algebra.h"
#include <vector>
using namespace std;

namespace wyGL
{
    struct Point3D 
    {
        vec4d coordinate_;
        vec3d normal_;
        vec3d color_;
        vec2d texture_;
        double oneOverZ_;
        vec2d textureOverZ_;
        Point3D(const vec4d& _coordinate, const vec3d& _normal = vec3d(0,0,0), 
            const vec3d& _color = vec3d(0,0,0), const vec2d& _texture = vec2d(0,0), const double _oneOverZ = 1,
            const vec2d& _textureOverZ = vec2d(0,0)):
        coordinate_(_coordinate), normal_(_normal), color_(_color), texture_(_texture)
        {

        }
        Point3D(){};
    };

    class Mesh
    {
    public:
        Mesh();
        Mesh(const vector<vec4d>& _points, const vector<vec2d>& _uvCoordinate, const vector<vec3i>& _indices);
        void reserve(unsigned int _size);
        void push_back(const Point3D& _p);

        Point3D& point(int i);
        const Point3D& point(int i) const;//const overloaded

        const int pointsSize() const;

        vector<vec3i>& getIndices(); 
        const vector<vec3i>& getIndices() const; // const overloaded

        vector<Point3D>& getPoints();


        void calculateFaceNormal();
        void calculateVertexNormal();

        void translate(const vec3d& _dis);
        void rotateY(const double _degree);
        //void translate(const vec3d& _displacement);

        // 
        void backCulling(const vec3d _cameraPosition);
        bool isCulled(unsigned int i);

        bool loadMesh(string _filename);

        // for debug convinienc ,I set it public
        
        // helper for culling
        //void clearCull();
    private:
        vector<Point3D> points_;
        vector<vec3i> indices_;
        static int rotateDegree;

        // normal computation helper
        vector<vec3d> faceNormals;
        vector<char> facesCulled;
        bool faceNormalComputed;

    };

}