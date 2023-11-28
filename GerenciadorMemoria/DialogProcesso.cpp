#include "pch.h"
#include "DialogProcesso.h"
#include "Controle.h"

IMPLEMENT_DYNAMIC(CDialogProcesso, CDialog)

CDialogProcesso::CDialogProcesso(CWnd* pParent)
    : CDialog(IDD_DLG_PROCESSO, pParent)
{
}

CDialogProcesso::~CDialogProcesso() {
}

void CDialogProcesso::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_LIST_PROCESSO, m_tabela);
}

BEGIN_MESSAGE_MAP(CDialogProcesso, CDialog)
END_MESSAGE_MAP()

BOOL CDialogProcesso::OnInitDialog()
{
    if (!__super::OnInitDialog())
        return FALSE;

    this->SetWindowTextA(m_sTitulo.c_str());

    CControle controle;
    int qtdThreads = controle.QtdThreadsProcesso(m_iPID);

    if (!controle.VerificaUsuarioAdmin())
        MessageBox("Voc� precisa ter privil�gios de administrador para abrir este processo.", "Aviso");

    controle.GetTipoMemoriaProcesso(m_iPID);
    controle.GetTempoProcessandoMiliSegundos(m_iPID);

    GetDlgItem(IDC_ST_QTD_THREADS)->SetWindowTextA(std::to_string(qtdThreads).c_str());
    GetDlgItem(IDC_STATIC_EXECUTANDO)->SetWindowTextA(std::to_string(controle.m_TempoExecutandoMiliSegundos).c_str());

    m_tabela.InsertColumn(0, "Tipo de mem�ria", LVCFMT_LEFT, 150);
    m_tabela.InsertColumn(1, "Endere�o", LVCFMT_LEFT, 150);
    m_tabela.InsertColumn(2, "Regi�o (Bytes)", LVCFMT_LEFT, 150);

    GetDlgItem(IDC_ST_WORKING_MAX)->SetWindowTextA(std::to_string(controle.m_pairWorkingSet.second).c_str());
    GetDlgItem(IDC_ST_WORKING_MIN)->SetWindowTextA(std::to_string(controle.m_pairWorkingSet.first).c_str());

    m_tabela.SetExtendedStyle(LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT);

    for (const auto& pTipoMemoria : controle.m_mapTypeMemoriaRegionSize)
    {
        std::string sTipoMemoria;
        switch (pTipoMemoria.first)
        {
            case MEM_PRIVATE: sTipoMemoria = "Privada"        ; break;
            case MEM_MAPPED : sTipoMemoria = "Mapeada"        ; break;
            case MEM_IMAGE  : sTipoMemoria = "Se��o de imagem"; break;
        }

        for (const auto& pRegiao : pTipoMemoria.second)
        {
            m_tabela.InsertItem(0, sTipoMemoria.c_str());
            m_tabela.SetItemText(0, 1, pRegiao.sAdressMemoria.c_str());
            m_tabela.SetItemText(0, 2, std::to_string(pRegiao.regionSize).c_str());
        }
    }

    return true;
}
