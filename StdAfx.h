
#pragma once
#pragma warning(disable : 4996) // no deprecated funcions warnings

#ifndef WINVER
#define WINVER _WIN32_WINNT_WIN7
#endif

#define VC_EXTRALEAN        // Exclude rarely-used stuff from Windows headers

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions
#include <afxdisp.h>        // MFC Automation classes
#include <afxcontrolbars.h>	// MFC support for ribbon and control bars
#include <afxole.h>         // MFC OLE 2.0 support
#include <afxodlgs.h>       // MFC OLE 2.0 dialog support
#include <afxcmn.h>         // MFC support for Windows Common Controls
#include <afxinet.h>
#include <afxsock.h>

#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxpriv.h>
#endif // _AFX_NO_AFXCMN_SUPPORT

#include <winspool.h>
#include <iphlpapi.h>
#include <strsafe.h>
#include <xmllite.h>

// STL headers
#include <algorithm>
#include <memory>
#include <vector>
#include <array>
#include <map>

// 3rd party
#include "zlib.h"
#ifdef UNITTEST
#include "gtest/gtest.h"
#endif

// Manam
#include "resource.h"
#include "ManConst.h"

#include "Flag.h"
#include "DrawObj.h"
#include "ManODPNET.h"

/* Napisy wyswietlane w GUI, ktore sa osadzone w kodzie powinny byc edytowane jako Windows 1250 */
