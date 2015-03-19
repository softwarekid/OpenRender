
// wyGL.h : main header file for the wyGL application
//
#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"       // main symbols


// CwyGLApp:
// See wyGL.cpp for the implementation of this class
//

class CwyGLApp : public CWinApp
{
public:
	CwyGLApp();


// Overrides
public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();

// Implementation
	afx_msg void OnAppAbout();
	DECLARE_MESSAGE_MAP()
};

extern CwyGLApp theApp;
