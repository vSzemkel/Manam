
#pragma once

class CDrawOpis final : public CDrawObj
{
    DECLARE_SERIAL(CDrawOpis);

// public: declared in macro
    CDrawOpis() noexcept {};
    CDrawOpis(const CRect& position) noexcept;
    CDrawOpis(const CRect& position, const TCHAR *tx) noexcept;

    virtual void Draw(CDC *pDC) override;
    virtual void Print(CDC *pDC) override;
    virtual void Serialize(CArchive &ar) override;

    virtual BOOL OnOpen(CDrawView *pView) override;
    virtual CDrawObj *Clone(CDrawDoc *pDoc) const override;

    int m_opi_xx;
    float m_Scale;

protected:
    friend class CRectTool;

private:
    static const CPoint drawrogi;
    static const CPoint printrogi;
    void DrawInternal(CDC *pDC, CRect& rect) const;
};
