#pragma once
#include <thread>

class CGerenciadorMemoriaDlg : public CDialogEx
{
public:
	CGerenciadorMemoriaDlg(CWnd* pParent = nullptr);

	enum { IDT_MYTIMER = 2 };

#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_GERENCIADORMEMORIA_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);

protected:
	HICON m_hIcon;

	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnLButtonDblClk(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	DECLARE_MESSAGE_MAP()

private:
	void CarregarInformacoes();

private:
	CListCtrl m_tabela;
	CProgressCtrl m_progressCtrl;
	CProgressCtrl m_progressCtrlRam;
};
