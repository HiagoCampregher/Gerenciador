#pragma once
#include <string>
#include <map>
#include <vector>

struct REG_INFO_PROCESSO
{
    std::string sNome;
    std::string sPrioridade;

    double privateMemoryMB;
};

struct REG_INFO_MEMORIA
{
    SIZE_T regionSize;
    std::string sAdressMemoria;
};

struct REG_INFO_MEMORIA_PAGINACAO
{
    size_t szMemoriaPaginada;
    size_t szTamanho;
    size_t szMemoriaNaoPaginada;
    float fMemoriaCache;
};

class CControle
{
public:
    void CarregaInformacoesIdentificacaoProcesso();

    std::map<ULONG, REG_INFO_PROCESSO> m_mapInfoProcessos;
    std::pair<SIZE_T, SIZE_T> m_pairWorkingSet;
    DWORDLONG m_TempoExecutandoMiliSegundos = 0;

    float GetDiscoDisponivel();
    float GetDiscoTotal();
    float GetRamDisponivel();
    float GetRamTotal();
    std::pair<SIZE_T, SIZE_T> GetWorkingSet(HANDLE hProcess);

    REG_INFO_MEMORIA_PAGINACAO GetInfoComplementarMemoria();

    bool VerificaUsuarioAdmin();

    void GetTempoProcessandoMiliSegundos(int idProcesso);

    int QtdThreadsProcesso(int idProcesso);
    void GetTipoMemoriaProcesso(DWORD dwProcessId);

private:
    BOOL IsUserAnAdmin();

public:
    std::map<DWORD, std::vector<REG_INFO_MEMORIA>> m_mapTypeMemoriaRegionSize;
};
