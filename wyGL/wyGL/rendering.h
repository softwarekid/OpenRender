#pragma  once
#include "stdafx.h"
#include <afxwin.h>
#include "linear_algebra.h"
#include "mesh.h"
#include "camera.h"
#include <vector>
using namespace std;

#define TEXTURE 1

namespace wyGL
{
    inline bool cmp(vec3i& a, vec3i& b)
    {
        return a[1] < b[1] || (a[1] == b[1] && a[0] < b[0]);
    }

    static enum shadeMode{wyGL_FLAT,wyGL_SMOOTH};
    static enum rasterizeMode{wyGL_LINES,wyGL_TRIANGLES};

    class WyShader
    {

    public:    
        //////////////////////////////////rasterization////////////////////////////
        struct Fragment
        {
            // the structure storing attributes of a point
            vec2i coordinate_; // x, y pixel on the screen
            vec3d color_;
            double depth_;
            double oneOverZ_;
            vec2d textureOverZ_;
            Fragment(const vec2i& _coordinate, const vec3d& _color, const double _depth, const double _oneOverZ, const vec2d& _textureOverZ):
                coordinate_(_coordinate),
                color_(_color),
                depth_(_depth),
                oneOverZ_(_oneOverZ),
                textureOverZ_(_textureOverZ)
            {}
            bool operator < (const Fragment& b)
            {
                return coordinate_[0] < b.coordinate_[0];
            }

            Fragment operator - (const Fragment& b)
            {
                return Fragment(coordinate_ - b.coordinate_, color_ - b.color_, depth_ - b.depth_, oneOverZ_ - b.oneOverZ_, textureOverZ_ - b.textureOverZ_);
            }
        };

        struct Edge
        {
            // for attributes interpolation
            double depthOfLowFrag;
            double deltaDepth;
            vec3d colorOfLowFrag;
            vec3d deltaColor;
            double oneOverZOfLowFrag;
            double deltaOneOverZ;
            vec2d textureOverZOfLowFrag;
            vec2d deltaTextureOverZ;


            // for pure scanline algorithm
            int nexty;
            int curx;
            int dx;
            int dy; 
            Edge *next;
        };

        struct XDepth
        {
            int x_;
            double depth_; 
            bool operator < (const XDepth& b)
            {
                return x_ < b.x_;
            }
        };

        WyShader(const int _xResolution, const int _yResolution);
        WyShader();
        ~WyShader();
        // for 2d rendering
        void render(const vector<Fragment>& _points, const vector<vec3i>& _triangleIndices, long* _pbitmap);
        // for 3d rendering
        void render(const Mesh& _mesh, Camera& _camera, long* _pbitmap);
        // naive DDA, no optimization
        void DDA(const vec2i& _point1, const vec2i& _point2, COLORREF _color, long* _pbitmap);

        // only fill a polygon 
        void scanline(const vector<Fragment>& _polygon, long* _pbitmap) ;

        // set shading and rasterizing mode
        void setShadeMode(const shadeMode _shadeMode);
        void setRasterizeMode(const rasterizeMode _rasterizeMode);

        void loadTexture();
        void drawPixel(int _x, int _y, double _oneOverZ, const vec2d& _textureOverZ, vec3d& _color, double _depth, long* _pbitmap);

        void setViewport(int width_, int height_);
        void clearDepth(double _depth);

        void enable(int _state);
        void disable(int _state);

    private:

        ///////////////////////////////scanline helper////////////////////////////

        vector<Edge*> scanlineArray_; 
        vector<vec2i> scanVertices_;  // vertices of the polygon to be scanned
        vector<vector<Fragment> > fragmentAcrossScanline_;

        // 
        void insert(const Fragment& _lowPt, const Fragment& _abovePt, const Fragment& _ajacentPtOfAbovePt);
        // calculate points intersected with each scanline for an edge
        void intersectedFrags(const int _line, const  Edge *_edge);


        //////////////////////////////////lighting////////////////////////////////



        //////////////////////////////////texture/////////////////////////////////
        void generateTexture();
        void uvUnify(vec2d& _uvCoord);



    private:
        shadeMode shadeMode_;
        rasterizeMode rasterizeMode_;
        int xResolution_;
        int yResolution_;

        // two buffers for each pixel
        vector<vector<COLORREF> > colorBuffer;
        vector<vector<double> > depthBuffer;

        // texture buffer
        vector<vector<wyGL::vec3i> > texture_;
        int textureWidth_;
        int textureHeight_;


        // state variables
        bool textureOn;

    };
}

