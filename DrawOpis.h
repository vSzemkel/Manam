
#pragma once

class CDrawOpis final : public CDrawObj
{
    DECLARE_SERIAL(CDrawOpis);

// public: declared in macro
    CDrawOpis() noexcept {};
    CDrawOpis(const CRect& position) noexcept;
    CDrawOpis(const CRect& position, const TCHAR *tx) noexcept;

    void Draw(CDC *pDC) override;
    void Print(CDC *pDC) override;
    void Serialize(CArchive &ar) override;

    BOOL OnOpen(CDrawView *pView) override;
    CDrawObj *Clone(CDrawDoc *pDoc) const override;

    int m_opi_xx;
    float m_Scale;

protected:
    friend class CRectTool;

private:
    static const CPoint drawrogi;
    static const CPoint printrogi;
    void DrawInternal(CDC *pDC, CRect& rect) const;
};
