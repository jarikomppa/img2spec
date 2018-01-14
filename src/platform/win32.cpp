#include "win32.h"

#include <windows.h>
#include <SDL.h>

int getFileDate(char *aFilename) {
    FILETIME ft;
    HANDLE f = 0;
    int iters = 0;
    while (f == 0 && iters < 16) {
        f = CreateFileA(aFilename, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
        if (!f)
            SDL_Delay(20);
        iters++;
    }
    if (f) {
        GetFileTime(f, 0, 0, &ft);
        CloseHandle(f);
    }
    return ft.dwHighDateTime ^ ft.dwLowDateTime;
}


const char *openDialog(const char *Title, const char *Filter) {

    OPENFILENAMEA ofn;
    static char szFileName[1024] = "";

    ZeroMemory(&ofn, sizeof(ofn));

    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = 0;
    ofn.lpstrFilter = Filter;

    ofn.nMaxFile = 1024;

    ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
    ofn.lpstrFile = szFileName;

    ofn.lpstrTitle = Title;

    if (GetOpenFileNameA(&ofn))
        return szFileName;
    else return nullptr;
}

const char *saveDialog(const char *Title, const char *Filter, const char *Ext) {

    OPENFILENAMEA ofn;
    static char szFileName[1024] = "";

    ZeroMemory(&ofn, sizeof(ofn));

    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = 0;
    ofn.lpstrFilter = Filter;

    ofn.nMaxFile = 1024;

    ofn.Flags = OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;
    ofn.lpstrFile = szFileName;

    ofn.lpstrTitle = Title;
    ofn.lpstrDefExt = Ext;

    if (GetSaveFileNameA(&ofn))
        return szFileName;
    else return nullptr;
}
