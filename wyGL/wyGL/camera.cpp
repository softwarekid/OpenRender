#include "stdafx.h"
#include "camera.h"
#include <math.h>
#define ZCLIPPINGPLANE -0.00001
// the following transformations and clipping can be referred to 
// Donald Hearn, etc. Computer Graphics with OpenGL (3rd edition), Chinese version
namespace wyGL
{
#define PI 3.14159265358979323846
    void wyGL::Camera::lookAt(const ExtrinsicParams& _exParams)
    {
        exParams_ = _exParams;
        // page 292
        vec3d n = exParams_.position_ - exParams_.centerPoint_;
        n.normalize();
        vec3d u = exParams_.upDirection_.crossProduct(n);

        u.normalize();
        vec3d v = n.crossProduct(u);     

        double rs = v.norm();
        // page 294
        for (int i = 0; i < 3; i++)
        {
            worldToCamera_(0,i) = u[i];
            worldToCamera_(1,i) = v[i];
            worldToCamera_(2,i) = n[i];
        }
        worldToCamera_(0,3) = - u.dotProduct(exParams_.position_);
        worldToCamera_(1,3) = - v.dotProduct(exParams_.position_);
        worldToCamera_(2,3) = - n.dotProduct(exParams_.position_);
        worldToCamera_(3,3) = 1;
    }

    void wyGL::Camera::perspective(const IntrinsicParams& _inParams)
    {
        // page 315
        inParams_ = _inParams;
        
        double yScale = 1 / tan(inParams_.fov_ * PI / 360);
        perspectiveTrans_(0,0) = yScale / inParams_.aspect_;
        perspectiveTrans_(1,1) = yScale;
        perspectiveTrans_(2,2) = (inParams_.near_ + inParams_.far_) / (inParams_.near_ - inParams_.far_);
        perspectiveTrans_(2,3) = (inParams_.near_ * inParams_.far_ * 2) / (inParams_.far_ - inParams_.near_);
        perspectiveTrans_(3,2) = -1;

        // clipping perspective matrix project the x, y to near plane,
        // while keep the z coordinate the same
        clipPerspective = perspectiveTrans_;
        clipPerspective(2, 2) = 1;
        clipPerspective(2, 3) = 0;
    }

    void wyGL::Camera::viewport(const int _width, const int _height)
    {
        // page 316
        matrix4d normViewVol;
        // pixel range is [0, 0] - [width - 1, height - 1]
        normViewVol(0,0) = (_width - 1) / 2.0;
        normViewVol(0,3) = (_width - 1) / 2.0;
        normViewVol(1,1) = (_height - 1) / 2.0;
        normViewVol(1,3) = (_height - 1) / 2.0;
        normViewVol(2,2) = 0.5;
        normViewVol(2,3) = 0.5;
        normViewVol(3,3) = 1;

        // for windows, origin of viewport is at left upper corner and the y direction is downward
        // this matrix is used to transform normal viewport to windows viewport, while keep z direction the same
        
        //matrix4d toWindowsViewPort;
        //toWindowsViewPort(0,0) = 1;
        //toWindowsViewPort(1,1) = -1;
        //toWindowsViewPort(1,3) = _height;
        //toWindowsViewPort(2,2) = 1;// to keep the sign of z the same
        //toWindowsViewPort(3,3) = 1;

        //viewPortTrans_ = toWindowsViewPort.rightMultiplyMatrix(normViewVol);
        viewPortTrans_ = normViewVol;
    }

    const matrix4d& Camera::getPerspectiveTrans()
    {
        return perspectiveTrans_;
    }

    const matrix4d& Camera::getViewPortTrans()
    {
        return viewPortTrans_;
    }

    void Camera::clipping(Mesh& _mesh)
    {
        // page 274
        for (int i = 0; i < _mesh.pointsSize(); i++)
        {
            Point3D& p = _mesh.point(i);
           // p.coordinate_ = p.coordinate_ / p.coordinate_[3];
            p.coordinate_ = p.coordinate_.leftMatrixMultiply(worldToCamera_);
            
        }

        viewZClip(_mesh);

        for (int i = 0; i < _mesh.pointsSize(); i++)
        {
            Point3D& p = _mesh.point(i);
            if (fabs(p.coordinate_[2]) < 0.00001)
            {
                p.coordinate_[2] = -0.00001;
            }
            p.oneOverZ_ = p.coordinate_[3] / p.coordinate_[2];
            p.textureOverZ_ = p.texture_ * p.oneOverZ_;
        }

        vector<Point3D> normPoints;
        normPoints.reserve(_mesh.pointsSize());
        for (int i = 0; i < _mesh.pointsSize(); i++)
        {
            normPoints.push_back(_mesh.point(i));
            normPoints[i].coordinate_ = _mesh.point(i).coordinate_.leftMatrixMultiply(clipPerspective);
        }

        // only tackle the situation that h > 0, h < 0 happens when \
        // the projected point is behind view point

        // only clip triangles 

        // can be optimized by reusing original vector
        vector<Point3D> tempPoints;
        tempPoints.reserve(_mesh.pointsSize());

        vector<vec3i>& indices = _mesh.getIndices();
        vector<vec3i> tempIndices;
        tempIndices.reserve(indices.size());

        for (int i = 0; i < indices.size(); i++)
        {
          /*  if (_mesh.isCulled(i))
            {
                continue;
            }*/

            vector<Point3D> newPoints;
            // a plane intersect with a cube will generate 6 intersection points at most
            newPoints.reserve(6);
            newPoints.push_back(normPoints[indices[i][0]]);
            newPoints.push_back(normPoints[indices[i][1]]);
            newPoints.push_back(normPoints[indices[i][2]]);

            vector<Point3D> tempNewPoints;
            tempNewPoints.reserve(6);

            // 6 clipping planes
            side clippingPlanes[6] = {Left, Right, Bottom, Top, Near, Far};
            for (int j = 0; j < 6; j++)
            {
                for (int k = 0; k < newPoints.size(); k++)
                {
                    Point3D nextPoint = newPoints[(k + 1) % newPoints.size()];
                    clipperProcess(newPoints[k], nextPoint, clippingPlanes[j], tempNewPoints);    
                }
                newPoints = tempNewPoints;
                tempNewPoints.clear();
            }

            if (0 == newPoints.size())
            {
                continue;
            }

            int base = tempPoints.size();
            for (int j = 0; j < newPoints.size(); j++)
            {
                tempPoints.push_back(newPoints[j]);
            }

            for (int j = 1; j < newPoints.size() - 1; j ++)
            {
                tempIndices.push_back(vec3i(base, base + j, base + j + 1));
            }
        }
        _mesh.getPoints() = tempPoints;
        _mesh.getIndices() = tempIndices;
    }

    bool Camera::isInside( const vec4d& _point, side _side )
    {
        vec4d tempPoint = _point;

        //// when point is behind view-point
        //if (tempPoint[3] < 0)
        //{
        //    tempPoint = vec4d(-tempPoint[0], -tempPoint[1], -tempPoint[2], -tempPoint[3]);
        //}

        switch (_side)
        {
        case Eye:
            if (_point[2] > ZCLIPPINGPLANE)
            {
                return false;
            }
            break;
        case Left:
            if (tempPoint[3] + tempPoint[0] < 0)
            {
                 return false;
            }
            break;
        case Right:
            if (tempPoint[3] - tempPoint[0] < 0)
            {
                return false;
            }
            break;
        case Bottom:
            if (tempPoint[3] + tempPoint[1] < 0)
            {
                return false;
            }
            break;
        case Top:
            if (tempPoint[3] - tempPoint[1] < 0)
            {
                return false;
            }
            break;
        case Near:
            if (tempPoint[2] > inParams_.near_)
            {
                return false;
            }
            break;
        case Far:
            if (tempPoint[2] < inParams_.far_)
            {
                return false;
            }
            break;
        }

        return true;
    }

    bool Camera::isInside(const vec4d& _point)
    {
        if (_point[2] > ZCLIPPINGPLANE)
        {
            return false;
        }
        return true;
    }

    wyGL::Point3D Camera::intersectionPoint( const Point3D& _startPt,const Point3D& _endPt, side _side )
    {
        vec4d startCoord = _startPt.coordinate_;
        vec4d endCoord = _endPt.coordinate_;
        double u = 0;
        double u1;
        Point3D result;
        switch (_side)
        {
        case Left:
            u1 = (-startCoord[3] * endCoord[3] - startCoord[0] * endCoord[3]) / (startCoord[3] * endCoord[0] - startCoord[0] * endCoord[3]);
            u = (startCoord[0] + startCoord[3]) / (startCoord[0] + startCoord[3] - endCoord[0] - endCoord[3]);
            break;
        case Right:
            u1 = (startCoord[3] * endCoord[3] - startCoord[0] * endCoord[3]) / (startCoord[3] * endCoord[0] - startCoord[0] * endCoord[3]);
            u = (startCoord[0] - startCoord[3]) / (startCoord[0] - startCoord[3] - endCoord[0] + endCoord[3]);
            break;
        case Bottom:
            u1 = (-startCoord[3] * endCoord[3] - startCoord[1] * endCoord[3]) / (startCoord[3] * endCoord[1] - startCoord[1] * endCoord[3]);
            u = (startCoord[1] + startCoord[3]) / (startCoord[1] + startCoord[3] - endCoord[1] - endCoord[3]);
            break;
        case Top:
            u1 = (startCoord[3] * endCoord[3] - startCoord[1] * endCoord[3]) / (startCoord[3] * endCoord[1] - startCoord[1] * endCoord[3]);
            u = (startCoord[1] - startCoord[3]) / (startCoord[1] - startCoord[3] - endCoord[1] + endCoord[3]);
            break;
        case Near:
            // special care is taken to near and far plane
            // I use "real" z to do linear interpolation
            //u = (1 / inParams_.near_ - _startPt.oneOverZ_) / (_endPt.oneOverZ_ - _startPt.oneOverZ_);
            u1 = (inParams_.near_ - startCoord[2]) / (endCoord[2] - startCoord[2]);
            break;
        case Far:
            //u = (1 / inParams_.far_ - _startPt.oneOverZ_) / (_endPt.oneOverZ_ - _startPt.oneOverZ_);
            u1 = (inParams_.far_ - startCoord[2]) / (endCoord[2] - startCoord[2]);
            break;
        default:
            u = 1;
            break;
        }
        
        if (_side == Near || _side == Far)
        {
            double startZ = 1 / _startPt.oneOverZ_;
            double endZ = 1 / _endPt.oneOverZ_;
            vec2d startTexture = vec2d(_startPt.textureOverZ_[0] /  _startPt.oneOverZ_,_startPt.textureOverZ_[1] /  _startPt.oneOverZ_);
            vec2d endTexture = vec2d(_endPt.textureOverZ_[0] /  _endPt.oneOverZ_,_endPt.textureOverZ_[1] /  _endPt.oneOverZ_);
            vec2d texture = startTexture + (( endTexture - startTexture) * u1);
            double z = startZ + (endZ - startZ) * u1;

            //vec2d startXYCoord = vec2d(_startPt.coordinate_[0] / _startPt.coordinate_[3],_startPt.coordinate_[1] / _startPt.coordinate_[3]);
            //vec2d endXYCoord = vec2d(_endPt.coordinate_[0] / _endPt.coordinate_[3], _endPt.coordinate_[1] / _endPt.coordinate_[3]);
            //vec2d intersectionCoord = startXYCoord + (endXYCoord - startXYCoord) * u;

            result.coordinate_ = _startPt.coordinate_ + ((_endPt.coordinate_ - _startPt.coordinate_) * u1);
            
            // color should be proportional to 1 / z
            result.oneOverZ_ = 1.0 / z;
            result.color_ = _startPt.color_ + ((_endPt.color_ - _startPt.color_) * u);
            result.textureOverZ_ = texture / z;
        }
        else
        {
            result.coordinate_ = _startPt.coordinate_ + ((_endPt.coordinate_ - _startPt.coordinate_) * u);
            result.color_ = _startPt.color_ + ((_endPt.color_ - _startPt.color_) * u);
            result.oneOverZ_ = _startPt.oneOverZ_ + ( (_endPt.oneOverZ_ - _startPt.oneOverZ_) * u1 );
            result.coordinate_[2] = 1 / result.oneOverZ_;
            result.textureOverZ_ = _startPt.textureOverZ_ + ( (_endPt.textureOverZ_ - _startPt.textureOverZ_) * u1);
        }
        return result;
    }

    wyGL::Point3D Camera::intersectionPoint(const Point3D& _startPt,const Point3D& _endPt)
    {
        vec4d startCoord = _startPt.coordinate_;
        vec4d endCoord = _endPt.coordinate_;
        double u = (ZCLIPPINGPLANE - startCoord[2]) / (endCoord[2] - startCoord[2]);

        Point3D result;
        result.coordinate_ = _startPt.coordinate_ +  ( (_endPt.coordinate_ - _startPt.coordinate_) * u );
        result.color_ = _startPt.color_ + ((_endPt.color_ - _startPt.color_) * u);
        result.texture_ = _startPt.texture_ + ((_endPt.texture_ - _startPt.texture_) * u);
        return result;
    }

    void Camera::clipperProcess( const Point3D& _startPt, const Point3D& _nextPt, const side _side, vector<Point3D>& _tempNewPoints )
    {
        if (isInside(_startPt.coordinate_, _side))
        {
            if (isInside(_nextPt.coordinate_, _side))
            {
                // in-in
                _tempNewPoints.push_back(_nextPt);
            }
            else
            {
                // in-out
                _tempNewPoints.push_back(intersectionPoint(_startPt, _nextPt, _side));
            }
        }
        else
        {
            if (isInside(_nextPt.coordinate_,_side))
            {
                // out-in
                _tempNewPoints.push_back(intersectionPoint(_startPt, _nextPt, _side));
                _tempNewPoints.push_back(_nextPt);
            }
            else
            {
                // out-out
                // do nothing
            }
        }
    }
    
    // handle z clipping under camera coordinate
    void Camera::clipperProcess(const Point3D& _startPt, const Point3D& _nextPt, vector<Point3D>& _tempNewPoints)
    {
        if (isInside(_startPt.coordinate_))
        {
            if (isInside(_nextPt.coordinate_))
            {
                // in-in
                _tempNewPoints.push_back(_nextPt);
            }
            else
            {
                // in-out
                _tempNewPoints.push_back(intersectionPoint(_startPt, _nextPt));
            }
        }
        else
        {
            if (isInside(_nextPt.coordinate_))
            {
                // out-in
                _tempNewPoints.push_back(intersectionPoint(_startPt, _nextPt));
                _tempNewPoints.push_back(_nextPt);
            }
            else
            {
                // out-out
                // do nothing
            }
        }
    }

    const vec3d Camera::getPostion() const
    { 
        return exParams_.position_;
    }

    void Camera::translate(const vec3d& _displacement)
    {
       vec3d xView = vec3d(worldToCamera_(0, 0), worldToCamera_(0, 1), worldToCamera_(0, 2));
       vec3d yView = vec3d(worldToCamera_(1, 0), worldToCamera_(1, 1), worldToCamera_(1, 2));
       vec3d zView = vec3d(worldToCamera_(2, 0), worldToCamera_(1, 1), worldToCamera_(2, 2));

       exParams_.position_ = exParams_.position_ + (xView * _displacement[0]) + (yView * _displacement[1]) + (zView * _displacement[2]);
       
       matrix4d disMatrix;
       disMatrix(0, 3) = -_displacement[0];
       disMatrix(1, 3) = -_displacement[1];
       disMatrix(2, 3) = -_displacement[2];
       disMatrix(0, 0) = 1;
       disMatrix(1, 1) = 1;
       disMatrix(2, 2) = 1;
       disMatrix(3, 3) = 1;

       worldToCamera_ = disMatrix.rightMultiplyMatrix(worldToCamera_);
    }

    void Camera::pitch(double _degree)
    {
        // rotate around x axis

        // camera's roll, yaw, pitch is conducted under camera coordinate frame
        // worldToCamera_ should guarantee every point is under camera coordinate frame after
        // applying the transformation, to keep it consistent, we should convert rotation transformation to frame transformation
        
        _degree = -_degree;

        /*
            1   0    0     0
            0   cos  -sin  0
            0   sin  cos   0
            0   0    0     1
        */

        matrix4d pitchTrans;
        double cs = cos(_degree);
        double sn = sin(_degree);
        pitchTrans(0, 0) = 1;
        pitchTrans(1, 1) = cs;
        pitchTrans(1, 2) = -sn;
        pitchTrans(2, 1) = sn;
        pitchTrans(2, 2) = cs;
        pitchTrans(3, 3) = 1;

        worldToCamera_ = pitchTrans.rightMultiplyMatrix(worldToCamera_);
    }

    void Camera::yaw(double _degree)
    {
        // rotate around y axis
         _degree = -_degree;

        /*
            cos  0    sin   0
            0    1    0     0
            -sin 0    cos   0
            0    0    0     1
        */

        matrix4d yawTrans;
        double cs = cos(_degree / 180.0 * PI);
        double sn = sin(_degree / 180.0 * PI);

        yawTrans(0, 0) = cs;
        yawTrans(0, 2) = sn;
        yawTrans(1, 1) = 1;
        yawTrans(2, 0) = -sn;
        yawTrans(2, 2) = cs;
        yawTrans(3, 3) = 1;
        worldToCamera_ = yawTrans.rightMultiplyMatrix(worldToCamera_);

     }

    void Camera::viewZClip(Mesh& _mesh)
    {
        // only tackle the situation that h > 0, h < 0 only happens when
        // the projected point is behind view point
        // can only clip triangles   

        // can be optimized by reusing original vector
        vector<Point3D> tempPoints;
        tempPoints.reserve(_mesh.pointsSize());

        vector<vec3i>& indices = _mesh.getIndices();
        vector<vec3i> tempIndices;
        tempIndices.reserve(indices.size());

        for (int i = 0; i < indices.size(); i++)
        {
            if (_mesh.isCulled(i))
            {
                continue;
            }

            vector<Point3D> newPoints;
            // a plane intersect with a cube will generate 6 intersection points at most
            newPoints.reserve(6);
            newPoints.push_back(_mesh.point(indices[i][0]));
            newPoints.push_back(_mesh.point(indices[i][1]));
            newPoints.push_back(_mesh.point(indices[i][2]));

            vector<Point3D> tempNewPoints;
            tempNewPoints.reserve(4);
           
            for (int k = 0; k < newPoints.size(); k++)
            {
                Point3D nextPoint = newPoints[(k + 1) % newPoints.size()];
                clipperProcess(newPoints[k], nextPoint, tempNewPoints);    
            }
            newPoints = tempNewPoints;
            
            if (0 == newPoints.size())
            {
                continue;
            }

            int base = tempPoints.size();
            for (int j = 0; j < newPoints.size(); j++)
            {
                tempPoints.push_back(newPoints[j]);
            }

            for (int j = 1; j < newPoints.size() - 1; j ++)
            {
                tempIndices.push_back(vec3i(base, base + j, base + j + 1));
            }
        }
        _mesh.getPoints() = tempPoints;
        _mesh.getIndices() = tempIndices;
        // only the faces not get culled will be processed, so now all the faces stored
        // in mesh is not culled.
        //_mesh.clearCull();
    }

}