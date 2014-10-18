#include "../dialogs.h"
#include <windows.h>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int Dialog_open(char_t* path)
{
	OPENFILENAME ofn;

	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.lpstrFile = path;
	ofn.lpstrFile[0] = '\0';
	ofn.nMaxFile = MAX_PATH;
	ofn.lpstrFilter = "All Files (*.*)\0*.*\0";
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

	return GetOpenFileName(&ofn);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int Dialog_save(char* path)
{
	OPENFILENAME dialog;
	ZeroMemory(&dialog, sizeof(dialog));
	dialog.lStructSize = sizeof(dialog);
	dialog.lpstrFilter = "All Files (*.*)\0*.*\0";
	dialog.lpstrFile = path;
	dialog.nMaxFile = MAX_PATH;
	dialog.Flags = OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;
	dialog.lpstrDefExt = "*";
	return GetSaveFileName(&dialog);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Dialog_showError(const char* text)
{
	MessageBox(NULL, text, "Error", MB_ICONERROR | MB_OK);
}

