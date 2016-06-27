#include "../dialogs.h"
//#include "core/core.h"
#include <uv.h>
#include <windows.h>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int Dialog_open(char* path) {
    OPENFILENAME ofn;
    const int size = 4096;
    wchar_t filename[size];

    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.lpstrFile = filename;
    ofn.lpstrFile[0] = '\0';
    ofn.nMaxFile = size;
    ofn.lpstrFilter = L"All Files (*.*)\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = NULL;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

    int state = GetOpenFileName(&ofn);

    uv_utf16_to_utf8(filename, size, path, 4096);

    return size;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int Dialog_save(char* path) {
    (void)path;
    return -1;
    /*
       OPENFILENAME dialog;
       ZeroMemory(&dialog, sizeof(dialog));
       dialog.lStructSize = sizeof(dialog);
       dialog.lpstrFilter = L"All Files (*.*)\0*.*\0";
       dialog.lpstrFile = (wchar_t*)path;	// hack hack
       dialog.nMaxFile = MAX_PATH;
       dialog.Flags = OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;
       dialog.lpstrDefExt = L"*";
       return GetSaveFileName(&dialog);
     */

}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int Dialog_selectDirectory(char* path) {
    (void)path;
    return -1;
    /*
       OPENFILENAME dialog;
       ZeroMemory(&dialog, sizeof(dialog));
       dialog.lStructSize = sizeof(dialog);
       dialog.lpstrFilter = L"All Files (*.*)\0*.*\0";
       dialog.lpstrFile = (wchar_t*)path;	// hack hack
       dialog.nMaxFile = MAX_PATH;
       dialog.Flags = OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;
       dialog.lpstrDefExt = L"*";
       return GetSaveFileName(&dialog);
     */
}

