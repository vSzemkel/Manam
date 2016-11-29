// cntritem.h : interface of the CDrawItem class
//
// This is a part of the Microsoft Foundation Classes C++ library.
// Copyright (C) 1992-1993 Microsoft Corporation
// All rights reserved.
//
// This source code is only intended as a supplement to the
// Microsoft Foundation Classes Reference and Microsoft
// QuickHelp and/or WinHelp documentation provided with the library.
// See these sources for detailed information regarding the
// Microsoft Foundation Classes product.

#pragma once

class CDrawDoc;
class CDrawView;

class CDrawItem : public COleClientItem
{
	DECLARE_SERIAL(CDrawItem)

// Constructors
public:
	CDrawItem(CDrawDoc* pContainer = nullptr, CDrawOleObj* pDrawObj = nullptr);
		// Note: pContainer is allowed to be NULL to enable IMPLEMENT_SERIALIZE
		//  IMPLEMENT_SERIALIZE requires the class have a constructor with
		//  zero arguments.  Normally, OLE items are constructed with a
		//  non-NULL document pointer.

// Attributes
public:
	CDrawDoc* GetDocument() const
		{ return (CDrawDoc*)COleClientItem::GetDocument(); }
	CDrawView* GetActiveView() const
		{ return (CDrawView*)COleClientItem::GetActiveView(); }

	CDrawOleObj* m_pDrawObj;    // back pointer to OLE draw object

// Operations
	BOOL UpdateExtent();

// Implementation
public:
	~CDrawItem();
#ifdef _DEBUG
	void AssertValid() const override;
	void Dump(CDumpContext& dc) const override;
#endif
	void Serialize(CArchive& ar) override;
	void OnGetItemPosition(CRect& rPosition) override;

protected:
	void OnChange(OLE_NOTIFICATION wNotification, DWORD dwParam) override;
	BOOL OnChangeItemPosition(const CRect& rectPos) override;
};

/////////////////////////////////////////////////////////////////////////////
