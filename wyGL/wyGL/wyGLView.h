
// wyGLView.h : interface of the CwyGLView class
//

#pragma once
#include <vector>
#include "linear_algebra.h"
#include "camera.h"
#include "mesh.h"
#include "material.h"
#include "rendering.h"
#include "light.h"
using namespace std;
class CwyGLView : public CView
{
protected: // create from serialization only
    CwyGLView();
    DECLARE_DYNCREATE(CwyGLView)

    // Attributes
public:
    CwyGLDoc* GetDocument() const;

    // Operations
public:

    // Overrides
public:
    virtual void OnDraw(CDC* pDC);  // overridden to draw this view

    virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:


    virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
    virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
    virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);


    // Implementation
public:
    virtual ~CwyGLView();
#ifdef _DEBUG
    virtual void AssertValid() const;
    virtual void Dump(CDumpContext& dc) const;
#endif

protected:

    // Generated message map functions
protected:
    DECLARE_MESSAGE_MAP()

private:
    // some containers to hold data

    vector<wyGL::vec4d> points3D_;
    vector<wyGL::vec2d> uvCoordinate_;
    vector<wyGL::vec3i> indices3D_;
    wyGL::Camera camera_;
    wyGL::Mesh cubeMesh_;
    wyGL::Mesh sphereMesh_;

    wyGL::Mesh origiCubeMesh_;
    wyGL::Mesh origiSphereMesh_;


    wyGL::Material material_;
    wyGL::Light light_;
    wyGL::WyShader shader_;

    int viewportWidth_;
    int viewportHeight_;



public:

    void initCube();
    void initSphere();
    void init();
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
    afx_msg BOOL OnEraseBkgnd(CDC* pDC);
    afx_msg void OnTimer(UINT_PTR nIDEvent);
};

#ifndef _DEBUG  // debug version in wyGLView.cpp
inline CwyGLDoc* CwyGLView::GetDocument() const
{ return reinterpret_cast<CwyGLDoc*>(m_pDocument); }
#endif

