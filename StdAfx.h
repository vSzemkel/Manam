
#pragma once
#pragma warning(disable : 4996) // no deprecated funcions warnings

#ifndef WINVER
#define WINVER 0x0600       // Windows Vista
#endif

#define VC_EXTRALEAN        // Exclude rarely-used stuff from Windows headers

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions
#include <afxdisp.h>        // MFC Automation classes
#include <afxcontrolbars.h>	// MFC support for ribbon and control bars
#include <afxole.h>         // MFC OLE 2.0 support
#include <afxodlgs.h>       // MFC OLE 2.0 dialog support
#include <afxcmn.h>         // MFC support for Windows Common Controls
#include <direct.h>
#include <time.h>
#include <math.h>
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxpriv.h>
#endif // _AFX_NO_AFXCMN_SUPPORT

#include <afxinet.h>
#include <afxsock.h>
#include <process.h>

#include <strsafe.h>

// STL headers
#include <algorithm>
#include <functional>
#include <memory>
#include <string>
#include <vector>
#include <array>
#include <map>

// Manam
#include "DrawObj.h"
#include "ManODPNET.h"
