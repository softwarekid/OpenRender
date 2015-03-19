#ifndef camera_h__
#define camera_h__
#include "linear_algebra.h"
#include "mesh.h"
#include <vector>

using namespace std;
namespace wyGL
{
    struct ExtrinsicParams
    {
        vec3d position_;
        vec3d centerPoint_;
        vec3d upDirection_;
        ExtrinsicParams() {}
        ExtrinsicParams(vec3d _position, vec3d _centerPoint, vec3d _upDirection):
            position_(_position),centerPoint_(_centerPoint),upDirection_(_upDirection)               
        {}
    };

    struct IntrinsicParams
    {
        double near_;
        double far_;
        double fov_;
        double aspect_;
        IntrinsicParams() {}
        IntrinsicParams(double _near, double _far, double _fov, double _aspect):
            near_(_near),far_(_far),fov_(_fov),aspect_(_aspect)
        {}
    };


    class Camera
    {
    public:
        enum side{Eye, Left, Right, Bottom, Top, Near, Far};
        Camera() {};

        void lookAt(const ExtrinsicParams& _exParams);

        // transform coordinates in camera frame to canonical perspective coordinates
        // reference point is at origin and view plane is coincide with near plane
        void perspective(const IntrinsicParams& _inParams);

        void viewport(const int _width, const int _height);

        const matrix4d& getPerspectiveTrans();

        const matrix4d& getViewPortTrans();
        const vec3d getPostion() const;

        void clipping(Mesh& _mesh);


        // camera movement
        // translate is conducted under world coordinate frame
        void translate(const vec3d& _displacement);
        // rotate around x
        void pitch(double _degree); 
        // rotate around y
        void yaw(double _degree); 

    private:
        ExtrinsicParams exParams_;
        IntrinsicParams inParams_;     
        matrix4d perspectiveTrans_;
        matrix4d viewPortTrans_;
        matrix4d worldToCamera_;
        matrix4d clipPerspective;

        /////////////////////////////clipping helper///////////////////////////////

        bool isInside(const vec4d& _point, side _side);
        Point3D intersectionPoint(const Point3D& _startPt,const Point3D& _endPt, side _side);
        void clipperProcess(const Point3D& _startPt, const Point3D& _nextPt, const side _side, vector<Point3D>& _tempNewPoints);
        


        // for point behind view point, we should z coordinate to less than 0
        void viewZClip(Mesh& _mesh);
        void clipperProcess(const Point3D& _startPt, const Point3D& _nextPt, vector<Point3D>& _tempNewPoints);
        bool isInside(const vec4d& _point);
        Point3D intersectionPoint(const Point3D& _startPt,const Point3D& _endPt);
    };
}
#endif // camera_h__
