
#pragma once

///////// CDrawOpis
class CDrawOpis final : public CDrawObj
{
protected:
    DECLARE_SERIAL(CDrawOpis);
    CDrawOpis();
    CDrawOpis(const CRect *position, const TCHAR *tx);

public:
    CDrawOpis(const CRect *position);
    int m_opi_xx;
    float m_Scale;
    // Implementation
public:
    virtual void Draw(CDC *pDC) override;
    virtual void Print(CDC *pDC) override;
    virtual void Serialize(CArchive &ar) override;

    virtual BOOL OnOpen(CDrawView *pView) override;
    virtual CDrawObj *Clone(CDrawDoc *pDoc) const override;

protected:
    friend class CRectTool;

private:
    static const CPoint drawrogi;
    static const CPoint printrogi;
    void DrawInternal(CDC *pDC, CRect& rect) const;
};
