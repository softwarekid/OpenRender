#include "stdafx.h"
#include "rendering.h"
#include <algorithm>
using namespace std;

namespace wyGL
{

    WyShader::WyShader(const int _xResolution, const int _yResolution):
        shadeMode_(wyGL_FLAT),
        rasterizeMode_(wyGL_LINES),
        xResolution_(_xResolution),
        yResolution_(_yResolution),
        colorBuffer(_xResolution + 1,vector<COLORREF>(_yResolution + 1,0)),
        depthBuffer(_xResolution + 1,vector<double>(_yResolution + 1,1)),
        textureOn(false)
    {
        // the range of y is [0, yResolution_], so should add 1
        //scanlineArray_.resize(yResolution_ + 1);
    }

    WyShader::WyShader()
    {
        shadeMode_ = wyGL_FLAT;
        rasterizeMode_ = wyGL_LINES;
    }

    void WyShader::DDA(const vec2i& _point1, const vec2i& _point2, COLORREF _color, long* _pbitmap) 
    {
        int dx = _point2[0] - _point1[0];
        int dy = _point2[1] - _point1[1];
        int steps;
        if(abs(dx) > abs(dy))
        {
            steps = abs(dx);
        }
        else
        {
            steps = abs(dy);
        }

        float xIncre = (float)dx / (float)steps;
        float yIncre = (float)dy / (float)steps;

        float x = _point1[0];
        float y = _point1[1];

        for(int k = 0; k < steps; k++)
        {
            int a = (int)(x + 0.5);
            int b = (int)(y + 0.5);
            // B G R
            *(_pbitmap + xResolution_ * b + a) = RGB(0, 0, 255);
            x += xIncre;
            y += yIncre;
        }
    }

    void WyShader::render(const vector<Fragment>& _fragments, const vector<vec3i>& _triangleIndices, long* _pbitmap) 
    {
        if (wyGL_LINES == rasterizeMode_)
        {
            for (int i = 0; i < _triangleIndices.size(); i++)
            {
                DDA(_fragments[_triangleIndices[i][0]].coordinate_, _fragments[_triangleIndices[i][1]].coordinate_,RGB(0,0,255),_pbitmap);
                DDA(_fragments[_triangleIndices[i][1]].coordinate_, _fragments[_triangleIndices[i][2]].coordinate_,RGB(0,0,255),_pbitmap);
                DDA(_fragments[_triangleIndices[i][2]].coordinate_, _fragments[_triangleIndices[i][0]].coordinate_,RGB(0,0,255),_pbitmap);
            }
        }
        else if (wyGL_TRIANGLES == rasterizeMode_)
        {
            vector<Fragment> polygon;
            polygon.reserve(3);
            for (int i = 0; i < _triangleIndices.size(); i++)
            {
                vec3i ind = _triangleIndices[i];
                polygon.clear();
                polygon.push_back(_fragments[ind[0]]);
                polygon.push_back(_fragments[ind[1]]);
                polygon.push_back(_fragments[ind[2]]);
                scanline(polygon,_pbitmap);
            }

            //for (int i = 0; i < _triangleIndices.size(); i++)
            //{
            //    DDA(_fragments[_triangleIndices[i][0]].coordinate_, _fragments[_triangleIndices[i][1]].coordinate_,RGB(0,0,255),_dc);
            //    DDA(_fragments[_triangleIndices[i][1]].coordinate_, _fragments[_triangleIndices[i][2]].coordinate_,RGB(0,0,255),_dc);
            //    DDA(_fragments[_triangleIndices[i][2]].coordinate_, _fragments[_triangleIndices[i][0]].coordinate_,RGB(0,0,255),_dc);
            //}
        }
    }

    void WyShader::render(const Mesh& _mesh, Camera& _camera, long* _pbitmap) 
    {
        matrix4d viewPortTrans = _camera.getViewPortTrans();

        vector<Fragment> fragment;
        fragment.reserve(_mesh.pointsSize());
        vec4d tempCoord;
        for (int i = 0; i < _mesh.pointsSize(); i++)
        {
            tempCoord = _mesh.point(i).coordinate_.leftMatrixMultiply(viewPortTrans);
            vec2i coordinate = vec2i(tempCoord[0] / tempCoord[3] + 0.5, tempCoord[1] / tempCoord[3] + 0.5);
            Fragment p(coordinate, _mesh.point(i).color_, tempCoord[2] / tempCoord[3], _mesh.point(i).oneOverZ_, _mesh.point(i).textureOverZ_);
            fragment.push_back(p);
        }

        render(fragment, _mesh.getIndices(), _pbitmap);
    }

    void WyShader::scanline(const vector<Fragment>& _polygon, long* _pbitmap) 
    {
        scanlineArray_.clear();
        scanlineArray_.resize(yResolution_ + 1);

        int n = _polygon.size();
        vector<vec3i> pointsWithIndex;
        pointsWithIndex.reserve(n);
        scanVertices_.clear();
        scanVertices_.reserve(n);
        for (int i = 0; i < _polygon.size(); i++)
        {
            scanVertices_.push_back(_polygon[i].coordinate_);
            pointsWithIndex.push_back(vec3i(_polygon[i].coordinate_[0], _polygon[i].coordinate_[1], i));
        }

        std::sort(pointsWithIndex.begin(), pointsWithIndex.end(), cmp);

        for (int i = 0; i < n; i++)
        {
            int ind = pointsWithIndex[i][2];
            int next = (ind + 1)% n;
            int pre = (ind - 1 + n) % n; 

            if (_polygon[next].coordinate_[1] > _polygon[ind].coordinate_[1])
            {
                insert(_polygon[ind], _polygon[next], _polygon[(next + 1) % n]);
            }

            if (_polygon[pre].coordinate_[1] > _polygon[ind].coordinate_[1]) 
            {
                insert(_polygon[ind], _polygon[pre], _polygon[(pre - 1 + n) % n]);
            }
        }

        vector<int> nEdgeIntersected(n,0);

        fragmentAcrossScanline_ = vector<vector<Fragment> >(yResolution_ + 1, vector<Fragment>());

        for (int i = 0; i <= yResolution_; i++)
        {
            Edge* edge = scanlineArray_[i];
            while (edge)
            {
                intersectedFrags(i, edge);
                edge = edge->next;
            }

           /* CPen newpen(PS_SOLID,1,RGB(0, 0, 0));
            _dc.SelectObject(&newpen);*/

            sort(fragmentAcrossScanline_[i].begin(), fragmentAcrossScanline_[i].end());
            for (int j = 0; j < fragmentAcrossScanline_[i].size(); j += 2) 
            {
                Fragment fragmentDiff = fragmentAcrossScanline_[i][j + 1] - fragmentAcrossScanline_[i][j];
                double dx = fragmentDiff.coordinate_[0];

                vec3d color = fragmentAcrossScanline_[i][j].color_;
                //double depth = fragmentAcrossScanline_[i][j].depth_;
                double oneOverZ = fragmentAcrossScanline_[i][j].oneOverZ_;
                vec2d textureOverZ = fragmentAcrossScanline_[i][j].textureOverZ_;

                if (fabs(dx) < 0.000001)
                {
                    vec2i coord = fragmentAcrossScanline_[i][j].coordinate_;
                    drawPixel(coord[0], coord[1], oneOverZ, textureOverZ,color,1 / oneOverZ ,_pbitmap);
                    continue;
                }

                vec3d deltaColor = vec3d(fragmentDiff.color_[0] / dx, fragmentDiff.color_[1] / dx, fragmentDiff.color_[2] / dx);
                double deltaDepth = fragmentDiff.depth_ / dx;
                double deltaOneOverZ = fragmentDiff.oneOverZ_ / dx;
                vec2d deltaTextureOverZ = vec2d( fragmentDiff.textureOverZ_[0] / dx, fragmentDiff.textureOverZ_[1] / dx);

                //int y = fragmentAcrossScanline_[i][j].coordinate_[1];
                for (int k = fragmentAcrossScanline_[i][j].coordinate_[0]; k <= fragmentAcrossScanline_[i][j+1].coordinate_[0]; k++)
                {
                    drawPixel(k, i, oneOverZ, textureOverZ,color,1 / oneOverZ,_pbitmap);
                   // depth = depth + deltaDepth;
                    color = color + deltaColor;
                    oneOverZ = oneOverZ + deltaOneOverZ;
                    textureOverZ = textureOverZ + deltaTextureOverZ;
                }
            }
        }

        // delete the allocated memory
        for (int i = 0; i < scanlineArray_.size(); i++)
        {
            Edge* edge = scanlineArray_[i];
            Edge *eNext;
            while (edge)
            {
                eNext = edge->next;
                delete edge;
                edge = eNext; 
            }
        }

    }

    void WyShader::insert(const Fragment& _lowFrag, const Fragment& _aboveFrag, const Fragment& _ajacentFragOfAbove)
    {
        int y = _aboveFrag.coordinate_[1];
        int y1 = _lowFrag.coordinate_[1];
        int y2 = _ajacentFragOfAbove.coordinate_[1];
        if (y1 > y2)
        {
            swap(y1, y2);
        }
        bool flag = false;
        if (y1 < y && y < y2)
        {
            // retreat a step in order to leave y to be handled by other scanline 
            flag = true;
        }

        int x = _lowFrag.coordinate_[0];
        y = _lowFrag.coordinate_[1];

        Edge* edge = scanlineArray_[y];
        Edge* preEdge = NULL;
        while (edge) 
        {
            // insert increasingly according y coordinate 
            if (edge->nexty > _aboveFrag.coordinate_[1] || (edge->nexty == _aboveFrag.coordinate_[1] && edge->curx > _aboveFrag.coordinate_[0])) 
            {
                break;
            }
            preEdge = edge;
            edge = edge->next;
        }

        Edge *tempEdge = new Edge;

        double dy = _aboveFrag.coordinate_[1] - _lowFrag.coordinate_[1];

        // depth to be interpolated
        tempEdge->depthOfLowFrag = _lowFrag.depth_;
        tempEdge->deltaDepth = (_aboveFrag.depth_ - _lowFrag.depth_) / dy;

        // color to be interpolated
        tempEdge->colorOfLowFrag = _lowFrag.color_;
        vec3d colorDiff = _aboveFrag.color_ - _lowFrag.color_;
        tempEdge->deltaColor = vec3d(colorDiff[0] / dy, colorDiff[1] / dy, colorDiff[2] / dy);

        // oneOverZ to be interpolated
        tempEdge->oneOverZOfLowFrag = _lowFrag.oneOverZ_;
        tempEdge->deltaOneOverZ = (_aboveFrag.oneOverZ_ - _lowFrag.oneOverZ_) / dy;

        // textureOverZ to be interpolated
        tempEdge->textureOverZOfLowFrag = _lowFrag.textureOverZ_;
        vec2d textureOverZDiff = _aboveFrag.textureOverZ_ - _lowFrag.textureOverZ_;
        tempEdge->deltaTextureOverZ = vec2d(textureOverZDiff[0] / dy, textureOverZDiff[1] / dy);


        // parameters to do scanline
        tempEdge->curx = x; 
        tempEdge->nexty = _aboveFrag.coordinate_[1];
        tempEdge->dx = _aboveFrag.coordinate_[0] - x; 
        tempEdge->dy = _aboveFrag.coordinate_[1] - y;
        tempEdge->next = NULL;
        if (flag) 
        {
            tempEdge->nexty--;
            //double u = (tempEdge->nexty - _lowFrag.coordinate_[1]) / (double)(_aboveFrag.coordinate_[1] - _lowFrag.coordinate_[1]);
        }

        if (preEdge) 
        {    
            tempEdge->next = preEdge->next;
            preEdge->next = tempEdge;
        }
        else 
        {
            tempEdge->next = scanlineArray_[y];
            scanlineArray_[y] = tempEdge;
        }
    }

    void WyShader::setShadeMode(const shadeMode _shadeMode)
    {
        shadeMode_ = _shadeMode;
    }

    void WyShader::setRasterizeMode(const rasterizeMode _rasterizeMode)
    {
        rasterizeMode_ = _rasterizeMode;
    }

    void WyShader::intersectedFrags(const int _line, const Edge *_edge)
    {
        int dx = _edge->dx;
        int dy = _edge->dy;

        bool flag = true;

        if (dx * dy < 0)
        {
            flag=0;
        }

        int cnt = 0;
        int x = _edge->curx;
        double depth = _edge->depthOfLowFrag;
        double deltaDepth = _edge->deltaDepth;

        vec3d color = _edge->colorOfLowFrag;
        vec3d deltaColor = _edge->deltaColor;

        double oneOverZ = _edge->oneOverZOfLowFrag;
        double deltaOneOverZ = _edge->deltaOneOverZ;

        vec2d textureOverZ = _edge->textureOverZOfLowFrag;
        vec2d deltaTextureOverZ = _edge->deltaTextureOverZ;

        for (int y = _line; y <= _edge->nexty; y++)
        {
            Fragment fragment(vec2i(x, y), color, depth, oneOverZ, textureOverZ);
            fragmentAcrossScanline_[y].push_back(fragment);

            cnt += 2 * abs(dx);
            while (cnt >= 2 * abs(dy)) 
            {
                cnt -= 2 * abs(dy);
                flag ? x++ : x--;
            }

            depth = depth + deltaDepth;
            color = color + deltaColor;
            oneOverZ = oneOverZ + deltaOneOverZ;
            textureOverZ = textureOverZ + deltaTextureOverZ;
        }
    }



    void WyShader::generateTexture()
    {
        textureWidth_ = 64;
        textureHeight_ = 64;
        texture_.resize(textureWidth_);
        int c;
        for (int i = 0; i < textureWidth_; i++)
        {
            texture_[i].reserve(textureHeight_);
            for (int j = 0; j < textureHeight_; j++)
            {
                c = ((((i&0x8)==0)^((j&0x8))==0))*255;
                vec3i color(c,c,c);
                texture_[i].push_back(color);
            }
        }
    }

    void WyShader::loadTexture()
    {
        generateTexture();
        textureOn = true;
    }

    void WyShader::uvUnify(vec2d& _uvCoord)
    {
        for (int i = 0; i < 2; i++)
        {
            if (fabs(_uvCoord[i] - 1) < 0.000001)
            {
                _uvCoord[i] = 1;
            }
            else if (fabs(_uvCoord[i]) < 0.000001)
            {
                _uvCoord[i] = 0;
            }
            else if(_uvCoord[i] - 1 > 0.000001)
            {
                _uvCoord[i] = _uvCoord[0] - (int)_uvCoord[0];
            }
        }
    }

    void WyShader::drawPixel(int _x, int _y, double _oneOverZ, const vec2d& _textureOverZ, vec3d& _color, double _depth, long* _pbitmap)
    {
        if (_depth - depthBuffer[_x][_y] > 0.00001)
        {
            vec2d uvCoord = vec2d(_textureOverZ[0] / _oneOverZ, _textureOverZ[1] / _oneOverZ);
            uvUnify(uvCoord);

            vec2i textureCoord = vec2i(uvCoord[0] * (textureWidth_ - 1), uvCoord[1] * (textureWidth_ - 1));
            vec3i textureIntColor =  texture_[textureCoord[0]][textureCoord[1]];
            vec3d textureDoubleColor = vec3d(textureIntColor[0] / 255.0, textureIntColor[1] / 255.0, textureIntColor[2] / 255.0);
            vec3d finalColor;
            if (textureOn)
            {
                finalColor = _color * textureDoubleColor;textureOn = true;
            }
            else
            {
                finalColor = _color;
            }

            assert(_y >= 0 && _y < yResolution_);
            assert(_x >= 0 && _x < xResolution_);
            *(_pbitmap + xResolution_ * _y + _x) = RGB(int(finalColor[2] * 255), int(finalColor[1] * 255), int(finalColor[0] * 255));
            //_dc.SetPixel(_x, _y, RGB(int(finalColor[0] * 255), int(finalColor[1] * 255), int(finalColor[2] * 255)));

            //colorBuffer[k][i] = RGB(color[0], color[1], color[2]);
            depthBuffer[_x][_y] = _depth;
        }
    }

    void WyShader::setViewport(int width_, int height_)
    {
        xResolution_ = width_;
        yResolution_ = height_;
        //colorBuffer = vector<> xResolution + 1,vector<COLORREF>(_yResolution + 1,0)),
        depthBuffer = vector<vector<double> >(xResolution_ + 1, vector<double>(yResolution_ + 1, 1));
    }

    void WyShader::clearDepth(double _depth)
    {
        for (int i = 0; i <= xResolution_; i++)
        {
            for (int j = 0; j <= yResolution_; j++)
            {
                depthBuffer[i][j] = _depth;
            }
        }
    }

    void WyShader::enable(int _state)
    {
        switch (_state)
        {
        case TEXTURE:
            textureOn = true;
            break;
        }
        
    }

    void WyShader::disable(int _state)
    {
        switch (_state)
        {
        case TEXTURE:
            textureOn = false;
            break;
        }
    }

    WyShader::~WyShader()
    {
        /*for (int i = 0; i < scanlineArray_.size(); i++)
        {
            Edge* edge = scanlineArray_[i];
            Edge *eNext;
            while (edge)
            {
                eNext = edge->next;
                delete edge;
                edge = eNext; 
            }
        }*/
    }
}