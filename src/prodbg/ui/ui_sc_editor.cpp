#include "ui_sc_editor.h"

#include <stdlib.h>
#include <stddef.h>
#include <algorithm>
#include <map>
#include <vector>
#include <string>

#include "scintilla/include/Platform.h"

#include "scintilla/src/CharClassify.h"
#include "scintilla/include/Scintilla.h"
#include "scintilla/include/ILexer.h"
#include "scintilla/src/SplitVector.h"
#include "scintilla/src/Partitioning.h"
#include "scintilla/src/RunStyles.h"
#include "scintilla/src/ContractionState.h"
#include "scintilla/src/CellBuffer.h"
#include "scintilla/src/KeyMap.h"
#include "scintilla/src/Indicator.h"
#include "scintilla/src/XPM.h"
#include "scintilla/src/LineMarker.h"
#include "scintilla/src/Style.h"
#include "scintilla/src/ViewStyle.h"
#include "scintilla/src/Decoration.h"
#include "scintilla/src/CharClassify.h"
#include "scintilla/src/CaseFolder.h"
#include "scintilla/src/Document.h"
#include "scintilla/src/Selection.h"
#include "scintilla/src/PositionCache.h"
#include "scintilla/src/EditModel.h"
#include "scintilla/src/MarginView.h"
#include "scintilla/src/EditView.h"
#include "scintilla/src/Editor.h"

//#include "scintilla/src/UniConversion.h"

//#include "scintilla/src/SciLexer.h"
//#include "scintilla/src/LexerModule.h"
//#include "scintilla/src/SciLexer.h"
//#include "scintilla/src/LexerModule.h"
//#include "scintilla/src/Catalogue.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct ScEditor : public Editor
{
public:
	void Initialise()
	{
	}

	void SetVerticalScrollPos()
	{
	}

	void SetHorizontalScrollPos()
	{
	}

	bool ModifyScrollBars(int nMax, int nPage)
	{
		(void)nMax;
		(void)nPage;
		return false;
	}

	void ClaimSelection()
	{
	}

	void Copy()
	{
	}

	void Paste()
	{
	}

	void NotifyChange()
	{
	}

	void NotifyParent(SCNotification scn)
	{
		(void)scn;
	}

	void CopyToClipboard(const SelectionText& selectedText)
	{
		(void)selectedText;
	}

	void SetMouseCapture(bool on)
	{
		(void)on;
	}

	bool HaveMouseCapture()
	{
		return false;
	}

	sptr_t DefWndProc(unsigned int iMessage, uptr_t wParam, sptr_t lParam)
	{
		return WndProc(iMessage, wParam, lParam);
	}
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ScEditor* ScEditor_create()
{
	ScEditor* ed = new ScEditor;

	return ed;
}


