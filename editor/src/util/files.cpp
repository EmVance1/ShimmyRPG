#include "pch.h"
#include "files.h"

#ifdef _WIN32
#include <Windows.h>
#include <shlobj.h>


int CALLBACK BrowseCallbackProc(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData) {
    if (uMsg == BFFM_INITIALIZED) {
        SendMessage(hwnd, BFFM_SETSELECTION, TRUE, lpData);
    }
    return 0;
}

std::optional<std::filesystem::path> select_folder(sf::WindowHandle handle) {
    // wchar_t buffer[512];
    // GetCurrentDirectory(512, buffer);
    // std::cout << std::filesystem::path(buffer).string() << "\n";

    BROWSEINFO bi = {0};
    bi.hwndOwner = handle;
    bi.lpszTitle = L"select scene";
    bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;
    // bi.lpfn = BrowseCallbackProc;
    // bi.lParam = (LPARAM)buffer;

    PIDLIST_ABSOLUTE pidl = SHBrowseForFolder(&bi);

    if (pidl != nullptr) {
        wchar_t path[MAX_PATH];
        if (SHGetPathFromIDList(pidl, path)) {
            return path;
        }
    }
    return {};
}

#else

std::optional<std::filesystem::path> select_folder(sf::WindowHandle handle) {
    return {}
}

#endif

