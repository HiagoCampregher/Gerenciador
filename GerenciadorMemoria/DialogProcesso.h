#pragma once
#include "resource.h"
#include <string>

class CDialogProcesso : public CDialog
{
    DECLARE_DYNAMIC(CDialogProcesso)

public:
    CDialogProcesso(CWnd * pParent = nullptr);
    virtual ~CDialogProcesso();

    virtual BOOL OnInitDialog() override;

#ifdef AFX_DESIGN_TIME
    enum { IDD = IDD_DLG_PROCESSO };
#endif

protected:
    virtual void DoDataExchange(CDataExchange * pDX);

    DECLARE_MESSAGE_MAP()

public:
    int m_iPID = 0;
    std::string m_sTitulo;

private:
    CListCtrl m_tabela;
};
