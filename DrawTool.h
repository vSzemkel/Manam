// drawtool.h - interface for CDrawTool and derivatives
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

#include "DrawObj.h"

class CDrawView;

class CDrawTool
{
    // Constructors
public:
    CDrawTool(DrawShape nDrawShape);

    // Overridables
    virtual void OnLButtonDown(CDrawView *pView, UINT nFlags, const CPoint& point);
    virtual void OnLButtonDblClk(CDrawView *pView, UINT nFlags, const CPoint& point);
    virtual void OnLButtonUp(CDrawView *pView, UINT nFlags, const CPoint& point);
    virtual void OnMouseMove(CDrawView *pView, UINT nFlags, const CPoint& point);

    // Attributes
    DrawShape m_drawShape;

    static CPoint c_down;
    static CPoint c_last;
    static UINT c_nDownFlags;
    static DrawShape c_drawShape;
    static std::vector<CDrawTool*> c_tools;

    static void OnCancel();
    static CDrawTool* FindTool(DrawShape drawShape);
};

class CSelectTool : public CDrawTool
{
    // Constructors
public:
    CSelectTool();
    // Implementation
    void OnLButtonDown(CDrawView *pView, UINT nFlags, const CPoint& point) override;
    void OnLButtonUp(CDrawView *pView, UINT nFlags, const CPoint& point) override;
    void OnMouseMove(CDrawView *pView, UINT nFlags, const CPoint& point) override;

    static int m_DragHandle;
private:
    CRect m_last_rect;
};

class CRectTool : public CDrawTool
{
    // Constructors
public:
    CRectTool(DrawShape drawShape);

    // Implementation
    void OnLButtonDown(CDrawView *pView, UINT nFlags, const CPoint& point) override;
    void OnLButtonDblClk(CDrawView *pView, UINT nFlags, const CPoint& point) override;
    void OnLButtonUp(CDrawView *pView, UINT nFlags, const CPoint& point) override;
    void OnMouseMove(CDrawView *pView, UINT nFlags, const CPoint& point) override;
};

class CKolorTool : public CDrawTool
{
    // Constructors
public:
    CKolorTool(DrawShape drawShape);
    // Implementation
    void OnLButtonDown(CDrawView *pView, UINT nFlags, const CPoint& point) override;
    void OnLButtonUp(CDrawView *pView, UINT nFlags, const CPoint& point) override;
    void OnMouseMove(CDrawView *pView, UINT nFlags, const CPoint& point) override;
};

class CLockTool : public CDrawTool
{
    // Constructors
public:
    CLockTool(DrawShape drawShape);
    // Implementation
    void OnLButtonDown(CDrawView *pView, UINT nFlags, const CPoint& point) override;
    void OnMouseMove(CDrawView *pView, UINT nFlags, const CPoint& point) override;
};

class CSpaceTool : public CDrawTool
{
    // Constructors
public:
    CSpaceTool(DrawShape nDrawShape);
    // Implementation
    void OnLButtonDown(CDrawView *pView, UINT nFlags, const CPoint& point) override;
    void OnLButtonUp(CDrawView *pView, UINT nFlags, const CPoint& point) override;
    void OnMouseMove(CDrawView *pView, UINT nFlags, const CPoint& point) override;
};
////////////////////////////////////////////////////////////////////////////
