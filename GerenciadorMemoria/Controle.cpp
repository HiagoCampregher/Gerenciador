#include "pch.h"
#include "Controle.h"
#include "stdafx.h"
#include <winnt.h>
#include "aclapi.h"
#include <sstream>

BOOL sm_EnableTokenPrivilege(LPCTSTR pszPrivilege)
{
    HANDLE hToken = 0;
    TOKEN_PRIVILEGES tkp = { 0 };

    if (!OpenProcessToken(GetCurrentProcess(),
        TOKEN_ADJUST_PRIVILEGES |
        TOKEN_QUERY, &hToken))
    {
        return FALSE;
    }

    if (LookupPrivilegeValue(NULL, pszPrivilege,
        &tkp.Privileges[0].Luid))
    {
        tkp.PrivilegeCount = 1;

        tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

        AdjustTokenPrivileges(hToken, FALSE, &tkp, 0, (PTOKEN_PRIVILEGES)NULL, 0);

        if (GetLastError() != ERROR_SUCCESS)
            return FALSE;

        return TRUE;
    }

    return FALSE;
}

typedef NTSTATUS(NTAPI* pfnNtQueryInformationProcess)(
    IN  HANDLE ProcessHandle,
    IN  PROCESSINFOCLASS ProcessInformationClass,
    OUT PVOID ProcessInformation,
    IN  ULONG ProcessInformationLength,
    OUT PULONG ReturnLength    OPTIONAL
    );

PVOID
QueryProcessInformation(
    IN HANDLE Process,
    IN PROCESSINFOCLASS ProcessInformationClass,
    IN DWORD ProcessInformationLength
) {
    sm_EnableTokenPrivilege(SE_DEBUG_NAME);

    PPROCESS_BASIC_INFORMATION pProcessInformation = NULL;
    pfnNtQueryInformationProcess gNtQueryInformationProcess;
    ULONG ReturnLength = 0;
    NTSTATUS Status;
    HMODULE hNtDll;

    if (!(hNtDll = LoadLibrary("ntdll.dll"))) {
        printf("Cannot load ntdll.dll.\n");
        return NULL;
    }

    if (!(gNtQueryInformationProcess = (pfnNtQueryInformationProcess)GetProcAddress(hNtDll, "NtQueryInformationProcess"))) {
        printf("Cannot load NtQueryInformationProcess.\n");
        return NULL;
    }

    if ((pProcessInformation = (PPROCESS_BASIC_INFORMATION)malloc(ProcessInformationLength)) == NULL) {
        printf("ExAllocatePoolWithTag failed.\n");
        return NULL;
    }

    if (!NT_SUCCESS(Status = gNtQueryInformationProcess(Process, ProcessInformationClass, pProcessInformation, ProcessInformationLength, &ReturnLength))) {
        printf("NtQueryInformationProcess should return NT_SUCCESS (Status = %#x).\n", Status);
        free(pProcessInformation);
        return NULL;
    }

    if (ReturnLength != ProcessInformationLength) {
        printf("Warning : NtQueryInformationProcess ReturnLength is different than ProcessInformationLength\n");
        return NULL;
    }

    DWORD dwPID = (DWORD)pProcessInformation->UniqueProcessId;
    DWORD dwPEBBaseAddress = (DWORD)pProcessInformation->PebBaseAddress;

    return pProcessInformation;
}

std::pair<SIZE_T, SIZE_T> CControle::GetWorkingSet(HANDLE hProcess)
{
    SIZE_T lpMinimumWorkingSetSize = 0;
    SIZE_T lpMaximumWorkingSetSize = 0;

    GetProcessWorkingSetSize(hProcess, &lpMinimumWorkingSetSize, &lpMaximumWorkingSetSize);

    return { lpMinimumWorkingSetSize, lpMaximumWorkingSetSize };
}

std::string GetFilePath(std::string sPathFile)
{
    int i = sPathFile.rfind('\\');
    return sPathFile.substr(i + 1, sPathFile.length() - i);
}

std::string ConvertAddressToString(LPVOID address) {
    std::stringstream stream;
    stream << "0x" << std::hex << reinterpret_cast<uintptr_t>(address);
    return stream.str();
}

void CControle::GetTipoMemoriaProcesso(DWORD dwProcessId)
{
    m_mapTypeMemoriaRegionSize.clear();

    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);

    MEMORY_BASIC_INFORMATION memInfo = {};

    HANDLE hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, TRUE, dwProcessId);

    m_pairWorkingSet = GetWorkingSet(hProcess);

    for (LPVOID addr = 0; addr < sysInfo.lpMaximumApplicationAddress; addr = (LPBYTE)memInfo.BaseAddress + memInfo.RegionSize)
    {
        if (!VirtualQueryEx(hProcess, addr, &memInfo, sizeof(memInfo)) == sizeof(memInfo))
            continue;

        if (!memInfo.RegionSize)
            break;

        if (memInfo.State != MEM_COMMIT)
            continue;

        auto pTipoMemoria = m_mapTypeMemoriaRegionSize.find(memInfo.Type);

        if (pTipoMemoria == m_mapTypeMemoriaRegionSize.end())
        {
            std::vector<REG_INFO_MEMORIA> aRegioes;
            REG_INFO_MEMORIA reg = {};
            reg.regionSize = memInfo.RegionSize;
            reg.sAdressMemoria = ConvertAddressToString(memInfo.BaseAddress);

            aRegioes.push_back(reg);

            m_mapTypeMemoriaRegionSize.insert({ memInfo.Type, aRegioes });
        }
        else
        {
            REG_INFO_MEMORIA reg = {};
            reg.regionSize = memInfo.RegionSize;
            reg.sAdressMemoria = ConvertAddressToString(memInfo.BaseAddress);

            pTipoMemoria->second.push_back(reg);
        }
    }
}

void CControle::CarregaInformacoesIdentificacaoProcesso()
{
    DWORD aProcesses[1024], cbNeeded, cProcesses;

    EnumProcesses(aProcesses, sizeof(aProcesses), &cbNeeded); // Retorna todos os processos que estão sendo executados

    cProcesses = cbNeeded / sizeof(DWORD);

    for (unsigned long long i = 0; i < cProcesses; i++)
    {
        HANDLE hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, TRUE, aProcesses[i]); // Abre o processo com objetivo de pegar as informações dele, passando a flag PROCESS_QUERY_LIMITED_INFORMATION

        if (hProcess == nullptr)
            continue;

        TCHAR szImageName[MAX_PATH] = TEXT("<unknown>");

        DWORD size = sizeof(szImageName);
        QueryFullProcessImageName(hProcess, 0, szImageName, &size);

        PPROCESS_BASIC_INFORMATION pProcessInformation = NULL;
        DWORD ProcessInformationLength = sizeof(PROCESS_BASIC_INFORMATION);

        if ((pProcessInformation = (PPROCESS_BASIC_INFORMATION)QueryProcessInformation(hProcess, ProcessBasicInformation, ProcessInformationLength)) == NULL)
            continue;

        //DWORD processCount;
        //DWORD returnLength;

        //NTSTATUS status = NtQuerySystemInformation(SystemProcessInformation, &processCount, sizeof(processCount), &returnLength);
        //if (NT_SUCCESS(status)) {
        //}
        //else {
        //}

        TCHAR szProcessName[MAX_PATH] = TEXT("<unknown>");
        std::string sPrioridade = "Desconhecida";

        if (GetProcessImageFileName(hProcess, szProcessName, MAX_PATH) != 0)
        {
            switch (GetPriorityClass(hProcess))
            {
            case IDLE_PRIORITY_CLASS:
                sPrioridade = "IDLE";
                break;
            case BELOW_NORMAL_PRIORITY_CLASS:
                sPrioridade = "BELOW_NORMAL";
                break;
            case NORMAL_PRIORITY_CLASS:
                sPrioridade = "NORMAL";
                break;
            case ABOVE_NORMAL_PRIORITY_CLASS:
                sPrioridade = "ABOVE_NORMAL";
                break;
            case HIGH_PRIORITY_CLASS:
                sPrioridade = "HIGH";
                break;
            case REALTIME_PRIORITY_CLASS:
                sPrioridade = "REALTIME";
                break;

            default:
                sPrioridade = "Prioridade não identificada.";
                break;
            }
        }

        double privateMemoryMB = 0;
        PROCESS_MEMORY_COUNTERS_EX pmc;
        if (GetProcessMemoryInfo(hProcess, (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc)))
        {
            SIZE_T privateMemoryUsage = pmc.PrivateUsage;
            privateMemoryMB = static_cast<double>(privateMemoryUsage) / (1024 * 1024);
        }

        REG_INFO_PROCESSO reg = {};
        reg.sNome = GetFilePath(szProcessName);
        reg.sPrioridade = sPrioridade;
        reg.privateMemoryMB = privateMemoryMB;

        m_mapInfoProcessos.insert({ aProcesses[i], reg });

        CloseHandle(hProcess);
    }
}

void CControle::GetTempoProcessandoMiliSegundos(int idProcesso)
{
    FILETIME creationTime, exitTime, kernelTime, userTime;

    HANDLE hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, TRUE, idProcesso);
    GetProcessTimes(hProcess, &creationTime, &exitTime, &kernelTime, &userTime);

    DWORDLONG startTime = creationTime.dwLowDateTime + creationTime.dwHighDateTime;
    DWORDLONG endTime = exitTime.dwLowDateTime + exitTime.dwHighDateTime;
    DWORDLONG totalTime = endTime - startTime;

    m_TempoExecutandoMiliSegundos = totalTime / 1000;
}

double ConverteByteEmGB(double bytes)
{
    return bytes / (1024 * 1024 * 1024);
};

REG_INFO_MEMORIA_PAGINACAO CControle::GetInfoComplementarMemoria()
{
    MEMORYSTATUSEX memoryStatus;
    memoryStatus.dwLength = sizeof(memoryStatus);

    REG_INFO_MEMORIA_PAGINACAO reg = {};

    PERFORMANCE_INFORMATION performanceInfo = {};
    if (GetPerformanceInfo(&performanceInfo, sizeof(performanceInfo)))
    {
        reg.fMemoriaCache = ConverteByteEmGB(performanceInfo.SystemCache);
        reg.szMemoriaPaginada = performanceInfo.CommitTotal;
        reg.szTamanho = performanceInfo.PageSize;
        reg.szMemoriaNaoPaginada = performanceInfo.CommitLimit - performanceInfo.CommitTotal;
    }

    return reg;
}

float CControle::GetRamDisponivel()
{
    MEMORYSTATUSEX statex;

    statex.dwLength = sizeof(statex);

    GlobalMemoryStatusEx(&statex);
    return (float)statex.ullAvailPhys / (1024 * 1024 * 1024);
}

float CControle::GetRamTotal()
{
    MEMORYSTATUSEX statex;

    statex.dwLength = sizeof(statex);

    GlobalMemoryStatusEx(&statex);
    return (float)statex.ullTotalPhys / (1024 * 1024 * 1024);
}

#define _1KB 1024

float ConverterBytesParaGB(ULONGLONG uiBytes)
{
    return uiBytes / std::pow(_1KB, 3);
}

float CControle::GetDiscoDisponivel()
{
    ULARGE_INTEGER uiFreeBytes = { 0 };
    ULARGE_INTEGER uiTotalBytes = { 0 };

    if (GetDiskFreeSpaceEx("C:\\", &uiFreeBytes, &uiTotalBytes, nullptr))
        return ConverterBytesParaGB(uiFreeBytes.QuadPart);

    return 0;
}

float CControle::GetDiscoTotal()
{
    ULARGE_INTEGER uiFreeBytes = { 0 };
    ULARGE_INTEGER uiTotalBytes = { 0 };

    if (GetDiskFreeSpaceEx("C:\\", &uiFreeBytes, &uiTotalBytes, nullptr))
        return ConverterBytesParaGB(uiTotalBytes.QuadPart);

    return 0;
}

bool CControle::VerificaUsuarioAdmin()
{
    return IsUserAnAdmin();
}

int CControle::QtdThreadsProcesso(int idProcesso)
{
    DWORD processId = idProcesso;
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);

    if (hSnapshot == INVALID_HANDLE_VALUE)
        return 0;

    THREADENTRY32 te32;
    te32.dwSize = sizeof(THREADENTRY32);
    DWORD threadCount = 0;

    if (Thread32First(hSnapshot, &te32)) {
        do {
            if (te32.th32OwnerProcessID == processId) {
                threadCount++;
            }
        } while (Thread32Next(hSnapshot, &te32));
    }

    CloseHandle(hSnapshot);

    return threadCount;
}

BOOL CControle::IsUserAnAdmin()
{
    BOOL fIsAdmin = FALSE;
    HANDLE hToken = NULL;

    if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken)) {
        DWORD dwSize = 0;

        TOKEN_INFORMATION_CLASS e = TokenElevationType;
        if (GetTokenInformation(hToken, e, &fIsAdmin, sizeof(fIsAdmin), &dwSize)) {
            return fIsAdmin;
        }
    }

    return FALSE;
}
