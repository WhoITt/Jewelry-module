
#pragma once

#ifndef __AFXWIN_H__
#error "включить pch.h до включения этого файла в PCH"
#endif

#include "resource.h"		// основные символы
#include "arxHeaders.h"
#include "AcExtensionModule.h"
#include "dbsol3d.h"     // AcDb3dSolid
#include "geassign.h"


class CMFCLibraryNrxProjectApp : public CWinApp
{
public:
	CMFCLibraryNrxProjectApp();

	// Переопределение
public:
	virtual BOOL InitInstance();

	DECLARE_MESSAGE_MAP()
};