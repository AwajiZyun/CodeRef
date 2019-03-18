
// VLCdemo.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// CVLCdemoApp:
// See VLCdemo.cpp for the implementation of this class
//

class CVLCdemoApp : public CWinApp
{
public:
	CVLCdemoApp();

// Overrides
public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()
};

extern CVLCdemoApp theApp;
