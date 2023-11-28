#include "pch.h"
#include "framework.h"
#include "GerenciadorMemoria.h"
#include "GerenciadorMemoriaDlg.h"
#include "afxdialogex.h"
#include "stdafx.h"

#include "Controle.h"
#include "DialogProcesso.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()

CGerenciadorMemoriaDlg::CGerenciadorMemoriaDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_GERENCIADORMEMORIA_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CGerenciadorMemoriaDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST1, m_tabela);
	DDX_Control(pDX, IDC_PROGESS_DISCO, m_progressCtrl);
	DDX_Control(pDX, IDC_PROGRESS_rAM, m_progressCtrlRam);
}

BEGIN_MESSAGE_MAP(CGerenciadorMemoriaDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_NOTIFY(NM_DBLCLK, IDC_LIST1, &CGerenciadorMemoriaDlg::OnLButtonDblClk)
	ON_WM_TIMER()
END_MESSAGE_MAP()

BOOL CGerenciadorMemoriaDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	m_tabela.InsertColumn(0, "PID", LVCFMT_LEFT, 50);
	m_tabela.InsertColumn(1, "Nome do processo", LVCFMT_LEFT, 200);
	m_tabela.InsertColumn(2, "Prioridade", LVCFMT_LEFT, 150);
	m_tabela.InsertColumn(3, "Memória privada (MB)", LVCFMT_LEFT, 150);
	 
	m_tabela.SetExtendedStyle(LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT);

	CarregarInformacoes();
	SetTimer(IDT_MYTIMER, 1000, NULL);

	return TRUE;
}

void CGerenciadorMemoriaDlg::CarregarInformacoes()
{
	 m_progressCtrl.SetRange(0, 100);
	 m_progressCtrlRam.SetRange(0, 100);

	CControle controle;
	float fDiscoDisponivel = controle.GetDiscoDisponivel();
	float fDiscoTotal = controle.GetDiscoTotal();

	GetDlgItem(IDC_ST_LIVRE_DISCO)->SetWindowTextA(std::to_string(fDiscoDisponivel).c_str());
	GetDlgItem(IDC_ST_TOTAL_DISCO)->SetWindowTextA(std::to_string(fDiscoTotal).c_str());

	float fRamDisponivel = controle.GetRamDisponivel();
	float fRamTotal = controle.GetRamTotal();

	REG_INFO_MEMORIA_PAGINACAO reg = controle.GetInfoComplementarMemoria();

	GetDlgItem(IDC_STATIC_PAGINACAO)->SetWindowTextA(std::to_string(reg.szMemoriaPaginada).c_str());
	GetDlgItem(IDC_STATIC_PAGINACAO_LIVRE)->SetWindowTextA(std::to_string(reg.szMemoriaNaoPaginada).c_str());
	GetDlgItem(IDC_STATIC_TAMANHO)->SetWindowTextA(std::to_string(reg.szTamanho).c_str());

	GetDlgItem(IDC_ST_LIVRE_RAM)->SetWindowTextA(std::to_string(controle.GetRamDisponivel()).c_str());
	GetDlgItem(IDC_ST_TOTAL_RAM)->SetWindowTextA(std::to_string(controle.GetRamTotal()).c_str());

	int iPercentualRam = (fRamTotal - fRamDisponivel) * 100 / fRamTotal;
	m_progressCtrlRam.SetPos(iPercentualRam);

	std::string sStaticPercentualRam = std::to_string(iPercentualRam) + "%";
	GetDlgItem(IDC_ST_PERCENTUAL_RAM)->SetWindowTextA(sStaticPercentualRam.c_str());

	int iPercentual = (fDiscoTotal - fDiscoDisponivel) * 100 / fDiscoTotal;
	m_progressCtrl.SetPos(iPercentual);

	std::string sStaticPercentualDisco = std::to_string(iPercentual) + "%";
	GetDlgItem(IDC_ST_PERCENTUAL_DISCO)->SetWindowTextA(sStaticPercentualDisco.c_str());

	controle.CarregaInformacoesIdentificacaoProcesso();
	for (const auto& pInfoProcesso : controle.m_mapInfoProcessos)
	{
		m_tabela.InsertItem(0, std::to_string(pInfoProcesso.first).c_str());
		m_tabela.SetItemText(0, 1, pInfoProcesso.second.sNome.c_str());
		m_tabela.SetItemText(0, 2, pInfoProcesso.second.sPrioridade.c_str());
		m_tabela.SetItemText(0, 3, std::to_string(pInfoProcesso.second.privateMemoryMB).c_str());
	}

	std::string sStaticQtdProcessos = "Qtd de processos: " + std::to_string(controle.m_mapInfoProcessos.size());
	GetDlgItem(IDC_ST_QTD_PROCESSOS)->SetWindowTextA(sStaticQtdProcessos.c_str());
}

void CGerenciadorMemoriaDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

void CGerenciadorMemoriaDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

HCURSOR CGerenciadorMemoriaDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CGerenciadorMemoriaDlg::OnLButtonDblClk(NMHDR* pNMHDR, LRESULT* pResult)
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;

	if (pNMListView->hdr.idFrom == IDC_LIST1 && pNMListView->hdr.code == NM_DBLCLK)
	{
		int itemSelecionado = pNMListView->iItem;
		int PID = atoi(m_tabela.GetItemText(itemSelecionado, 0));

		CDialogProcesso dlgProcesso;
		dlgProcesso.m_iPID = PID;
		dlgProcesso.m_sTitulo = m_tabela.GetItemText(itemSelecionado, 1);
		dlgProcesso.DoModal();
	}
}

void CGerenciadorMemoriaDlg::OnTimer(UINT_PTR nIDEvent)
{
	if (nIDEvent == IDT_MYTIMER)
	{
		CarregarInformacoes();
		return;
	}

	CDialogEx::OnTimer(nIDEvent);
}
