#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <string>
#include <vector>

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int nCmdShow) {
    // 1. Get path to root executable directory
    wchar_t exePath[MAX_PATH];
    GetModuleFileNameW(NULL, exePath, MAX_PATH);

    std::wstring dir(exePath);
    size_t lastSlash = dir.find_last_of(L"\\/");
    if (lastSlash != std::wstring::npos) {
        dir = dir.substr(0, lastSlash);
    }

    std::wstring distDir = dir + L"\\dist";
    std::wstring targetExe = distDir + L"\\soniq_app.exe";

    // 2. Prepend dist directory to PATH so all Qt6 and runtime DLLs resolve automatically
    wchar_t oldPath[32768];
    DWORD pathLen = GetEnvironmentVariableW(L"PATH", oldPath, 32768);
    std::wstring newPath = distDir + L";" + (pathLen > 0 ? oldPath : L"");
    SetEnvironmentVariableW(L"PATH", newPath.c_str());

    // 3. Launch the application process
    STARTUPINFOW si;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESHOWWINDOW;
    si.wShowWindow = static_cast<WORD>(nCmdShow);

    PROCESS_INFORMATION pi;
    ZeroMemory(&pi, sizeof(pi));

    std::wstring cmdLine = L"\"" + targetExe + L"\"";
    std::vector<wchar_t> cmdBuf(cmdLine.begin(), cmdLine.end());
    cmdBuf.push_back(0);

    if (CreateProcessW(
        targetExe.c_str(),
        cmdBuf.data(),
        NULL,
        NULL,
        FALSE,
        0,
        NULL,
        distDir.c_str(),
        &si,
        &pi)) {
        CloseHandle(pi.hThread);
        CloseHandle(pi.hProcess);
        return 0;
    }

    // Diagnostic alert if dist folder or binary was moved
    std::wstring errMsg = L"Could not launch SONIQ engine.\nPlease verify that:\n" + targetExe + L"\nexists.";
    MessageBoxW(NULL, errMsg.c_str(), L"SONIQ Launcher", MB_ICONERROR | MB_OK);
    return 1;
}
