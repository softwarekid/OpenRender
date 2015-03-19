
// wyGLView.cpp : implementation of the CwyGLView class
//

#include "stdafx.h"
// SHARED_HANDLERS can be defined in an ATL project implementing preview, thumbnail
// and search filter handlers and allows sharing of document code with that project.
#ifndef SHARED_HANDLERS
#include "wyGL.h"
#endif

#define TIMER1 1
#include "wyGLDoc.h"
#include "wyGLView.h"
#include "rendering.h"
#include "camera.h"
#include "mesh.h"
#include "light.h"
#include "material.h"
#include <time.h>
#include <iostream>
#ifdef _DEBUG
#define new DEBUG_NEW

#endif


// CwyGLView

IMPLEMENT_DYNCREATE(CwyGLView, CView)

    BEGIN_MESSAGE_MAP(CwyGLView, CView)
        // Standard printing commands
        ON_COMMAND(ID_FILE_PRINT, &CView::OnFilePrint)
        ON_COMMAND(ID_FILE_PRINT_DIRECT, &CView::OnFilePrint)
        ON_COMMAND(ID_FILE_PRINT_PREVIEW, &CView::OnFilePrintPreview)
        ON_WM_SIZE()
        ON_WM_KEYDOWN()
        ON_WM_ERASEBKGND()
        ON_WM_TIMER()
    END_MESSAGE_MAP()

    // CwyGLView construction/destruction

    CwyGLView::CwyGLView()
    {
        // TODO: add construction code here
        initCube();
        initSphere();
        init();
    }

    CwyGLView::~CwyGLView()
    {
    }

    BOOL CwyGLView::PreCreateWindow(CREATESTRUCT& cs)
    {
        // TODO: Modify the Window class or styles here by modifying
        //  the CREATESTRUCT cs


        return CView::PreCreateWindow(cs);
    }

    // CwyGLView drawing

    void CwyGLView::OnDraw(CDC* pDC)
    {
        static int startDrawingTime = clock();
        
        CwyGLDoc* pDoc = GetDocument();
        ASSERT_VALID(pDoc);
        if (!pDoc)
            return;
        // TODO: add draw code for native data here
        CClientDC dc(this);
        CRect rc;
        GetClientRect(&rc);            
        CDC memDC;
        memDC.CreateCompatibleDC(&dc);

        void * pbits32=NULL; 
        BITMAPINFOHEADER RGB32BITSBITMAPINFO={sizeof(BITMAPINFOHEADER),rc.Width(),rc.Height(),1,32,BI_RGB,0,0,0,0,0};
        HBITMAP hbm32=CreateDIBSection(memDC,(BITMAPINFO *)&RGB32BITSBITMAPINFO,DIB_RGB_COLORS,&pbits32,NULL,0);    

        if (NULL == hbm32)
        {

            exit(EXIT_FAILURE);
        }

        BITMAP bm32;
        GetObject(hbm32,sizeof(bm32),&bm32);
        while (bm32.bmWidthBytes%4) bm32.bmWidthBytes++;
        // select bitmap
        memDC.SelectObject(hbm32);

        long *pBitmap = (long *)bm32.bmBits;
        double farPlane = -500;

        shader_.setViewport(viewportWidth_, viewportHeight_);
        shader_.clearDepth(farPlane);
        camera_.perspective(wyGL::IntrinsicParams(-0.5, farPlane, 60 , viewportWidth_ / (double)viewportHeight_));
        camera_.viewport(viewportWidth_, viewportHeight_);

        cubeMesh_ = origiCubeMesh_;
        cubeMesh_.rotateY((clock() - startDrawingTime) / 30.0);


        // for sphere rendering
        sphereMesh_ = origiSphereMesh_;
        sphereMesh_.rotateY((clock() - startDrawingTime) / 30.0);
        sphereMesh_.translate(vec3d(-3, 3, 0));

        clock_t startTime  = clock();

        // for cube rendering
        light_.lighting(cubeMesh_, material_, camera_.getPostion());
        cubeMesh_.backCulling(camera_.getPostion());
        camera_.clipping(cubeMesh_);
        shader_.render(cubeMesh_, camera_, pBitmap);

        // for sphere rendering
        light_.lighting(sphereMesh_, material_, camera_.getPostion());
        sphereMesh_.backCulling(camera_.getPostion());
        camera_.clipping(sphereMesh_);

        shader_.disable(TEXTURE);
        shader_.render(sphereMesh_, camera_, pBitmap);
        shader_.enable(TEXTURE);

        clock_t finishTime = clock();
        double time_elapse  = (double)(finishTime - startTime) / CLOCKS_PER_SEC;
        std::cout<<finishTime<< endl;

        // copy bitmap to screen, act like swap
        dc.BitBlt(0, 0, rc.Width(), rc.Height(), &memDC, 0, 0, SRCCOPY);
        DeleteObject(hbm32);
        memDC.DeleteDC();
    }

    // CwyGLView printing

    BOOL CwyGLView::OnPreparePrinting(CPrintInfo* pInfo)
    {
        // default preparation
        return DoPreparePrinting(pInfo);
    }

    void CwyGLView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
    {
        // TODO: add extra initialization before printing
    }

    void CwyGLView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
    {
        // TODO: add cleanup after printing
    }


    // CwyGLView diagnostics

#ifdef _DEBUG
    void CwyGLView::AssertValid() const
    {
        CView::AssertValid();
    }

    void CwyGLView::Dump(CDumpContext& dc) const
    {
        CView::Dump(dc);
    }

    CwyGLDoc* CwyGLView::GetDocument() const // non-debug version is inline
    {
        ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CwyGLDoc)));
        return (CwyGLDoc*)m_pDocument;
    }


#endif //_DEBUG


    // CwyGLView message handlers

    void CwyGLView::OnSize(UINT nType, int cx, int cy)
    {
        CView::OnSize(nType, cx, cy);

        // TODO: Add your message handler code here
        viewportWidth_ = cx;
        viewportHeight_ = cy;
        SetTimer(TIMER1, 20, NULL);
    }
  

    void CwyGLView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
    {
        // TODO: Add your message handler code here and/or call default
        bool validKey = false;
        switch (nChar)
        {
        case 'W':
            camera_.translate(vec3d(0, 0, -0.1));
            validKey = true;
            break;
        case 'S':
            camera_.translate(vec3d(0, 0, 0.1));
            validKey = true;
            break;
        case 'Q':
            camera_.translate(vec3d(0, 0.1, 0));
            validKey = true;
            break;
        case 'E':
            camera_.translate(vec3d(0, -0.1, 0));
            validKey = true;
            break;
        case 'A':
            camera_.yaw(2);
            validKey = true;
            break;
        case 'D':
            camera_.yaw(-2);
            validKey = true;
            break;
        default:

            break;
        }
       /* if (true)
        {
            CRect rect;
            GetClientRect(&rect);
            InvalidateRect(rect);
            CView::OnKeyDown(nChar, nRepCnt, nFlags);
        }*/
    }


    void CwyGLView::initCube()
    {
        //vector<wyGL::vec2i> points2D;
        //vector<wyGL::vec3i> indices2D;
        //points2D.push_back(wyGL::vec2i(20,300));
        //points2D.push_back(wyGL::vec2i(300,100));
        //points2D.push_back(wyGL::vec2i(20,20));
        //indices2D.push_back(wyGL::vec3i(0,1,2));

        // test for scanline
        //points3D.push_back(wyGL::vec4d(20,300,100,1));
        //points3D.push_back(wyGL::vec4d(300,100,50,1));
        //points3D.push_back(wyGL::vec4d(20,20,20,1));
        //indices3D.push_back(wyGL::vec3i(0,1,2));

        points3D_.reserve(24);
        uvCoordinate_.reserve(24);
        indices3D_.reserve(12);
        // front 0 1 2 3
        uvCoordinate_.push_back(wyGL::vec2d(0.0, 0.0));
        points3D_.push_back(wyGL::vec4d(-1.0, -1.0, 1.0, 1));
        uvCoordinate_.push_back(wyGL::vec2d(1.0, 0.0));
        points3D_.push_back(wyGL::vec4d(1.0, -1.0, 1.0, 1));
        uvCoordinate_.push_back(wyGL::vec2d(1.0, 1.0));
        points3D_.push_back(wyGL::vec4d(1.0, 1.0, 1.0, 1));
        uvCoordinate_.push_back(wyGL::vec2d(0.0, 1.0));
        points3D_.push_back(wyGL::vec4d(-1.0, 1.0, 1.0, 1));

        // back 4 5 6 7
        uvCoordinate_.push_back(wyGL::vec2d(1.0, 0.0));
        points3D_.push_back(wyGL::vec4d(-1.0, -1.0, -1.0, 1));
        uvCoordinate_.push_back(wyGL::vec2d(1.0, 1.0));
        points3D_.push_back(wyGL::vec4d(-1.0, 1.0, -1.0, 1));
        uvCoordinate_.push_back(wyGL::vec2d(0.0, 1.0));
        points3D_.push_back(wyGL::vec4d(1.0, 1.0, -1.0, 1));
        uvCoordinate_.push_back(wyGL::vec2d(0.0, 0.0));
        points3D_.push_back(wyGL::vec4d(1.0, -1.0, -1.0, 1));

        // top 8 9 10 11
        uvCoordinate_.push_back(wyGL::vec2d(0.0, 1.0));
        points3D_.push_back(wyGL::vec4d(-1.0, 1.0, -1.0, 1));
        uvCoordinate_.push_back(wyGL::vec2d(0.0, 0.0));
        points3D_.push_back(wyGL::vec4d(-1.0, 1.0, 1.0, 1));
        uvCoordinate_.push_back(wyGL::vec2d(1.0, 0.0));
        points3D_.push_back(wyGL::vec4d(1.0, 1.0, 1.0, 1));
        uvCoordinate_.push_back(wyGL::vec2d(1.0, 1.0));
        points3D_.push_back(wyGL::vec4d(1.0, 1.0, -1.0, 1));

        // bottom 12 13 14 15
        uvCoordinate_.push_back(wyGL::vec2d(1.0, 1.0));
        points3D_.push_back(wyGL::vec4d(-1.0, -1.0, -1.0, 1));
        uvCoordinate_.push_back(wyGL::vec2d(0.0, 1.0));
        points3D_.push_back(wyGL::vec4d(1.0, -1.0, -1.0, 1));
        uvCoordinate_.push_back(wyGL::vec2d(0.0, 0.0));
        points3D_.push_back(wyGL::vec4d(1.0, -1.0, 1.0, 1));
        uvCoordinate_.push_back(wyGL::vec2d(1.0, 0.0));
        points3D_.push_back(wyGL::vec4d(-1.0, -1.0, 1.0, 1));

        // right 16 17 18 19
        uvCoordinate_.push_back(wyGL::vec2d(1.0, 0.0));
        points3D_.push_back(wyGL::vec4d(1.0, -1.0, -1.0, 1));
        uvCoordinate_.push_back(wyGL::vec2d(1.0, 1.0));
        points3D_.push_back(wyGL::vec4d(1.0, 1.0, -1.0, 1));
        uvCoordinate_.push_back(wyGL::vec2d(0.0, 1.0));
        points3D_.push_back(wyGL::vec4d(1.0, 1.0, 1.0, 1));
        uvCoordinate_.push_back(wyGL::vec2d(0.0, 0.0));
        points3D_.push_back(wyGL::vec4d(1.0, -1.0, 1.0, 1));

        // left 20 21 22 23
        uvCoordinate_.push_back(wyGL::vec2d(0.0, 0.0));
        points3D_.push_back(wyGL::vec4d(-1.0, -1.0, -1.0, 1));
        uvCoordinate_.push_back(wyGL::vec2d(1.0, 0.0));
        points3D_.push_back(wyGL::vec4d(-1.0, -1.0, 1.0, 1));
        uvCoordinate_.push_back(wyGL::vec2d(1.0, 1.0));
        points3D_.push_back(wyGL::vec4d(-1.0, 1.0, 1.0, 1));
        uvCoordinate_.push_back(wyGL::vec2d(0.0, 1.0));
        points3D_.push_back(wyGL::vec4d(-1.0, 1.0, -1.0, 1));

        // front
        indices3D_.push_back(wyGL::vec3i(0, 1, 2));
        indices3D_.push_back(wyGL::vec3i(0, 2, 3));

        // back
        indices3D_.push_back(wyGL::vec3i(4, 5, 6));
        indices3D_.push_back(wyGL::vec3i(4, 6, 7));

        // top
        indices3D_.push_back(wyGL::vec3i(8, 9, 10));
        indices3D_.push_back(wyGL::vec3i(8, 10, 11));

        // bottom
        indices3D_.push_back(wyGL::vec3i(12, 13, 14));
        indices3D_.push_back(wyGL::vec3i(12, 14, 15));

        // right
        indices3D_.push_back(wyGL::vec3i(16, 17, 18));
        indices3D_.push_back(wyGL::vec3i(16 ,18, 19));

        //// left
        indices3D_.push_back(wyGL::vec3i(20 ,21 ,22));
        indices3D_.push_back(wyGL::vec3i(20 ,22 ,23));

        cubeMesh_ = Mesh(points3D_, uvCoordinate_, indices3D_);        
        cubeMesh_.calculateVertexNormal();
    }

    void CwyGLView::initSphere()
    {
        sphereMesh_.loadMesh("../models/sphere.obj");
        sphereMesh_.calculateVertexNormal();

    }

    void CwyGLView::init()
    {
        camera_.lookAt(wyGL::ExtrinsicParams(wyGL::vec3d(-2.89, 3.26, 1.6), wyGL::vec3d(0,0,0), wyGL::vec3d(0,1,0)));
        light_.setPosition(vec4d(4.0, -4.0, -10.0, 0.0));
        shader_.loadTexture();
        shader_.setRasterizeMode(wyGL::wyGL_TRIANGLES);
        origiCubeMesh_ = cubeMesh_;
        origiSphereMesh_ = sphereMesh_;
    }

    BOOL CwyGLView::OnEraseBkgnd(CDC* pDC)
    {
        // TODO: Add your message handler code here and/or call default
        return true;
        // return CView::OnEraseBkgnd(pDC);
    }


    void CwyGLView::OnTimer(UINT_PTR nIDEvent)
    {
        // TODO: Add your message handler code here and/or call default
        CRect rect;

        switch(nIDEvent)
        {
        case TIMER1:
            //origiCubeMesh_.rotateY(10);
            GetClientRect(&rect);
            InvalidateRect(rect);
            break;
        default:
            MessageBox(_T("default: KillTimer"));
            KillTimer(nIDEvent);
            break;
        }

        // CView::OnTimer(nIDEvent);
    }
