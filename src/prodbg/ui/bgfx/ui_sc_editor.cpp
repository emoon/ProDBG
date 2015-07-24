#include "ui_sc_editor.h"
#include "core/file.h"
#include "api/include/pd_keys.h"

#include <stdlib.h>
#include <stddef.h>
#include <algorithm>
#include <cstring>
#include <map>
#include <vector>
#include <string>
#include <assert.h>

#include "scintilla/include/Platform.h"
#include "scintilla/include/Scintilla.h"
#include "scintilla/include/ILexer.h"
#include "scintilla/include/SciLexer.h"
#include "scintilla/src/lexlib/LexerModule.h"
#include "scintilla/src/lexlib/PropSetSimple.h"
#include "scintilla/src/CharClassify.h"
#include "scintilla/src/SplitVector.h"
#include "scintilla/src/Partitioning.h"
#include "scintilla/src/RunStyles.h"
#include "scintilla/src/Catalogue.h"
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
#include "scintilla/src/AutoComplete.h"
#include "scintilla/src/CallTip.h"
#include "scintilla/src/ScintillaBase.h"

#include "tinyxml2/tinyxml2.h"

#include <imgui.h>

#define TESTING_TOOLTIP_LOGIC 0
// TODO: Temp testing code
#if TESTING_TOOLTIP_LOGIC && !BX_PLATFORM_OSX
extern "C" __declspec(dllimport) void __stdcall OutputDebugStringA(const char* _str);
#endif

extern struct WindowImpl* AllocateWindowImpl();

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool htmlToColour(ColourDesired& colour, const char* html)
{
#if 1 // Built In
    colour.Set(html);
    colour.Set(colour.AsLong() | ((unsigned int)0xFF << 24)); // ColourDesired is lame in that it never sets the alpha channel..
    return true;
#else
    const char* start = html;
    if (*start == '#')
        ++start;

    const size_t size = strlen(start);
    if (size == 6)
    {
        // 8 bits per channel
        char* end;
        char parse[3];
        parse[2] = '\0';

        // Red
        parse[0] = start[0];
        parse[1] = start[1];
        unsigned int r = strtol(parse, &end, 16);

        // Green
        parse[0] = start[2];
        parse[1] = start[3];
        unsigned int g = strtol(parse, &end, 16);

        // Blue
        parse[0] = start[4];
        parse[1] = start[5];
        unsigned int b = strtol(parse, &end, 16);

        // ColourDesired is lame in that it never sets the alpha channel..
        long result = r | (g << 8) | (b << 16) | (0xFF << 24);
        colour.Set(result);
        return true;
    }
    else if (size == 3)
    {
        // 4 bits per channel
        char* end;
        char parse[2];
        parse[2] = '\0';

        // Red
        parse[0] = start[0];

        unsigned int r = strtol(parse, &end, 16);
        r = r * 16 + r;

        // Green
        parse[0] = start[1];
        unsigned int g = strtol(parse, &end, 16);
        g = g * 16 + g;

        // Blue
        parse[0] = start[2];
        unsigned int b = strtol(parse, &end, 16);
        b = b * 16 + b;

        // ColourDesired is lame in that it never sets the alpha channel..
        long result = r | (g << 8) | (b << 16) | (0xFF << 24);
        colour.Set(result);
        return true;
    }

    // Invalid color
    return false;
#endif
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static const char* breakpoint_xpm[] = {
    "16 16 72 1",
    "   c None",
    ".	c #772B1A",
    "+	c #903B2D",
    "@	c #F17A78",
    "#	c #C65C55",
    "$	c #F17876",
    "%	c #FF7E7E",
    "&	c #FF7B7B",
    "*	c #C65750",
    "=	c #8F3A2C",
    "-	c #EF7573",
    ";	c #FD7B7B",
    ">	c #FD7878",
    ",	c #FD7575",
    "'	c #FD7272",
    ")	c #C5524B",
    "!	c #8E392C",
    "~	c #E7716F",
    "{	c #F47676",
    "]	c #F47373",
    "^	c #F47070",
    "/	c #F46D6D",
    "(	c #F46A6A",
    "_	c #F46767",
    ":	c #C04C45",
    "<	c #8B382A",
    "[	c #DB6B69",
    "}	c #E77070",
    "|	c #E76D6D",
    "1	c #E76A6A",
    "2	c #E76868",
    "3	c #E76565",
    "4	c #E76161",
    "5	c #E75F5F",
    "6	c #E75D5D",
    "7	c #B8453E",
    "8	c #8D392B",
    "9	c #DB6867",
    "0	c #E56C6C",
    "a	c #E56A6A",
    "b	c #E56767",
    "c	c #E56363",
    "d	c #E56161",
    "e	c #E55E5E",
    "f	c #E55C5C",
    "g	c #E55959",
    "h	c #B8443D",
    "i	c #8A3729",
    "j	c #CD5D5B",
    "k	c #D66060",
    "l	c #D65E5E",
    "m	c #D65A5A",
    "n	c #D65858",
    "o	c #D65656",
    "p	c #D65353",
    "q	c #AF413A",
    "r	c #863326",
    "s	c #BB514F",
    "t	c #C25252",
    "u	c #C25050",
    "v	c #C24E4E",
    "w	c #C24C4C",
    "x	c #A33C36",
    "y	c #813023",
    "z	c #A54241",
    "A	c #AA4444",
    "B	c #AA4242",
    "C	c #953730",
    "D	c #7B2D1F",
    "E	c #893432",
    "F	c #822F28",
    "G	c #772B1D",
    "       .        ",
    "      ...       ",
    "     ..+..      ",
    "    ..+@#..     ",
    "   ..+$%&*..    ",
    "  ..=-;>,')..   ",
    " ..!~{]^/(_:..  ",
    "..<[}|1234567.. ",
    "..890abcdefgh.. ",
    " ..ijklmnopq..  ",
    "  ..rstuvwx..   ",
    "   ..yzABC..    ",
    "    ..DEF..     ",
    "     ..G..      ",
    "      ...       ",
    "       .        "
};

// C++ keywords
static const char cppKeyWords[] =

// Standard
    "asm auto bool break case catch char class const "
    "const_cast continue default delete do double "
    "dynamic_cast else enum explicit extern false finally "
    "float for friend goto if inline int long mutable "
    "namespace new operator private protected public "
    "register reinterpret_cast register return short signed "
    "sizeof static static_cast struct switch template "
    "this throw true try typedef typeid typename "
    "union unsigned using virtual void volatile "
    "wchar_t while "

// Extended
    "__asm __asume __based __box __cdecl __declspec "
    "__delegate delegate depreciated dllexport dllimport "
    "event __event __except __fastcall __finally __forceinline "
    "__int8 __int16 __int32 __int64 __int128 __interface "
    "interface __leave naked noinline __noop noreturn "
    "nothrow novtable nullptr safecast __stdcall "
    "__try __except __finally __unaligned uuid __uuidof "
    "__virtual_inheritance";

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// GW-WIP: For now we directly parse and translate from Eclipse color themes.

class ScEclipseTheme
{
public:
    ScEclipseTheme();
    ~ScEclipseTheme();

    bool Load(const char* themeName);
    void Apply(ScEditor* editor, int fontSize = -1, const char* fontName = nullptr);

private:
    bool LoadColour(ColourDesired& colour, const char* name, tinyxml2::XMLElement* element);

private:
    //char m_themeName[64];
    unsigned int m_themeId;

    ColourDesired m_searchResultIndication;
    ColourDesired m_filteredSearchResultIndication;
    ColourDesired m_occurrenceIndication;
    ColourDesired m_writeOccurrenceIndication;
    ColourDesired m_findScope;
    ColourDesired m_sourceHoverBackground;
    ColourDesired m_singleLineComment;
    ColourDesired m_multiLineComment;
    ColourDesired m_commentTaskTag;
    ColourDesired m_javadoc;
    ColourDesired m_javadocLink;
    ColourDesired m_javadocTag;
    ColourDesired m_javadocKeyword;
    ColourDesired m_class;
    ColourDesired m_interface;
    ColourDesired m_method;
    ColourDesired m_methodDeclaration;
    ColourDesired m_bracket;
    ColourDesired m_number;
    ColourDesired m_string;
    ColourDesired m_operator;
    ColourDesired m_keyword;
    ColourDesired m_annotation;
    ColourDesired m_staticMethod;
    ColourDesired m_localVariable;
    ColourDesired m_localVariableDeclaration;
    ColourDesired m_field;
    ColourDesired m_staticField;
    ColourDesired m_staticFinalField;
    ColourDesired m_deprecatedMember;
    ColourDesired m_background;
    ColourDesired m_currentLine;
    ColourDesired m_foreground;
    ColourDesired m_lineNumber;
    ColourDesired m_selectionBackground;
    ColourDesired m_selectionForeground;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct ScEditor : public ScintillaBase
{
private:
    int m_width;
    int m_height;
    int m_wheelVRotation;
    int m_wheelHRotation;

public:

    ScEditor()
        : m_width(0)
        , m_height(0)
        , m_wheelVRotation(0)
        , m_wheelHRotation(0)
    {
        memset(&interface, 0, sizeof(interface));
    }

    virtual ~ScEditor()
    {
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void Update()
    {
        //HandleInput();
        Tick();
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void ToggleBreakpoint()
    {
        Point caretPosition = PointMainCaret();
        unsigned int lineNumber = (unsigned int)LineFromLocation(caretPosition);
        std::vector<unsigned int>::iterator iter = find(m_breakpointLines.begin(), m_breakpointLines.end(), lineNumber);

        if (iter != m_breakpointLines.end())
        {
            // Breakpoint already exists
            m_breakpointLines.erase(iter);
            SendCommand(SCI_MARKERDELETE, lineNumber /* line number */, 0 /* marker id */);
        }
        else
        {
        	printf("marker add %d\n", lineNumber);
            // Breakpoint added
            m_breakpointLines.push_back(lineNumber);
            SendCommand(SCI_MARKERADD, lineNumber /* line number */, 0 /* marker id */);
        }
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    bool IsComment(int position)
    {
        //position = max(0, position - 1);
        sptr_t style = SendCommand(SCI_GETSTYLEAT, (uptr_t)position);

        // TODO: How to map this cleanly?
        return style == 2;
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    int GetWordStartPosition(int position, bool onlyWordCharacters)
    {
        return (int)SendCommand(SCI_WORDSTARTPOSITION, (uptr_t)position, (sptr_t)onlyWordCharacters);
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    int GetWordEndPosition(int position, bool onlyWordCharacters)
    {
        return (int)SendCommand(SCI_WORDENDPOSITION, uptr_t(position), sptr_t(onlyWordCharacters));
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    char* GetTextRange(int startPosition, int endPosition)
    {
        if (endPosition < startPosition)
        {
            int temp = startPosition;
            startPosition = endPosition;
            endPosition = temp;
        }

        int length = endPosition - startPosition;
        if (!length)
            return nullptr;

        char* result = static_cast<char*>(malloc(sizeof(char) * (size_t)length + 1));

        Sci_TextRange textRange;
        textRange.lpstrText = result;
        textRange.chrg.cpMin = startPosition;
        textRange.chrg.cpMax = endPosition;

        SendCommand(SCI_GETTEXTRANGE, 0, sptr_t(&textRange));
        result[length] = '\0';

        return result;
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    char* GetWordFromPosition(int position, int& start, int& end)
    {
        end   = GetWordEndPosition(position, true);
        start = GetWordStartPosition(position, true);
        return GetTextRange(start, end);
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    static bool IsKeyPressedMap(ImGuiKey key, bool repeat = false)
    {
        ImGuiIO& io = ImGui::GetIO();
        const int key_index = io.KeyMap[key];
        return ImGui::IsKeyPressed(key_index, repeat);
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void HandleInput()
    {
        // TODO: Would be better to decouple ImGui key values here and abstract it into a prodbg api instead
        if (IsKeyPressedMap(ImGuiKey_DownArrow, true))
        {
        	ImGui::SetScrollY(ImGui::GetScrollY() + 23); 
            Editor::KeyDown(SCK_DOWN /*SCK_NEXT*/, false, false, false);
        }
        else if (IsKeyPressedMap(ImGuiKey_UpArrow, true))
        {
        	ImGui::SetScrollY(ImGui::GetScrollY() - 23); 
            Editor::KeyDown(SCK_UP /*SCK_PRIOR*/, false, false, false);
        }
        else if (IsKeyPressedMap(ImGuiKey_V, false))
        {
            ToggleBreakpoint();
        }

        ImGuiIO& io = ImGui::GetIO();
        if (ImGui::IsMouseClicked(0))
        {
            // Left mouse button click
            Point pt = Point::FromInts((int)io.MouseClickedPos[0].x, (int)io.MouseClickedPos[0].y);

        #if TESTING_TOOLTIP_LOGIC
            int position = PositionFromLocation(pt, false, true);

            int wordStart;
            int wordEnd;
            if (char* result = GetWordFromPosition(position, wordStart, wordEnd))
            {
                char resultText[256];
                sprintf(resultText, "Word [%s] is %s\n", IsComment(position) ? "comment" : "normal", result);
            #if BX_PLATFORM_OSX
                printf(resultText);
            #else
                OutputDebugStringA(resultText);
            #endif
                CallTipShow(pt, result);

                free(result);
            }
        #else
            ButtonDown(pt, (unsigned int)io.MouseDownDuration[0], false, false, false);
        #endif
        }
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void HandleMouseWheel(const PDMouseWheelEvent& wheelEvent)
    {
        int topLineNew = topLine;
        int lines;
        int xPos = xOffset;
        int pixels;

        if (wheelEvent.wheelAxis == PDWHEEL_AXIS_HORIZONTAL)
        {
            m_wheelHRotation += wheelEvent.rotation * (wheelEvent.columnsPerRotation * vs.spaceWidth);
            pixels = m_wheelHRotation / wheelEvent.wheelDelta;
            m_wheelHRotation -= pixels * wheelEvent.wheelDelta;
            if (pixels != 0)
            {
                xPos += pixels;
                PRectangle rcText = GetTextRectangle();
                if (xPos > scrollWidth - (int)rcText.Width())
                    xPos = scrollWidth - (int)rcText.Width();
                HorizontalScrollTo(xPos);
            }
        }
        else if (wheelEvent.keyFlags & PDKEY_CTRL)
        {
            if (wheelEvent.rotation > 0)
                KeyCommand(SCI_ZOOMIN);
            else
                KeyCommand(SCI_ZOOMOUT);
        }
        else
        {
            short delta = wheelEvent.wheelDelta;
            if (!delta)
                delta = 120;
            m_wheelVRotation += wheelEvent.rotation;
            lines = m_wheelVRotation / delta;
            m_wheelVRotation -= lines * delta;
            if (lines != 0)
            {
                bool isPageScroll = (wheelEvent.keyFlags & PDKEY_SHIFT);
                if (isPageScroll)
                    lines = lines * LinesOnScreen();  // lines is either +1 or -1
                else
                    lines *= wheelEvent.linesPerRotation;
                topLineNew -= lines;
                ScrollTo(topLineNew);
            }
        }
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void Render()
    {
        PRectangle rcPaint = GetClientRectangle();

        AutoSurface surfaceWindow(this);
        if (surfaceWindow)
        {
            Paint(surfaceWindow, rcPaint);
            surfaceWindow->Release();
        }
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void SetAStyle(int style, ColourDesired fore, ColourDesired back = 0xFFFFFFFF, int size = -1, const char* face = nullptr)
    {
        SendCommand(SCI_STYLESETFORE, uptr_t(style), fore.AsLong());
        SendCommand(SCI_STYLESETBACK, uptr_t(style), back.AsLong());
        if (size >= 1)
            SendCommand(SCI_STYLESETSIZE, uptr_t(style), size);
        if (face)
            SendCommand(SCI_STYLESETFONT, uptr_t(style), reinterpret_cast<sptr_t>(face));
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void Initialise()
    {
        wMain = AllocateWindowImpl();

        // TODO: TEMP! Hook up properly to ImGui
        ImGuiIO& io = ImGui::GetIO();
        wMain.SetPosition(PRectangle::FromInts(0, 0, int(io.DisplaySize.x), int(io.DisplaySize.y)));

        // We need to disable buffered draw so Scintilla doesn't keep a yoffset of 0
        // when rendering text, thinking we are blitting through a pixmap. We want a
        // single draw list for efficiency.
        view.bufferedDraw = false;

        SendCommand(SCI_SETLEXER, SCLEX_CPP);
        SendCommand(SCI_SETSTYLEBITS, 7);

        SendCommand(SCI_SETKEYWORDS, 0, reinterpret_cast<sptr_t>(cppKeyWords));

        ScEclipseTheme scTheme;
        //bool result = scTheme.Load("data/themes/theme-1.xml");    // Oblivion
        bool result = scTheme.Load("data/themes/theme-118.xml");  // Wombat
        //bool result = scTheme.Load("data/themes/theme-383.xml");  // Sunburst
        //bool result = scTheme.Load("data/themes/theme-3796.xml"); // Ambients
        //bool result = scTheme.Load("data/themes/theme-4967.xml"); // Sublime Monokai
        //bool result = scTheme.Load("data/themes/theme-6563.xml"); // Monokai 2 Dark
        assert(result);

        const int fontSize = 13;
        const char* fontName = "data/font/source_code_pro/SourceCodePro-Medium.ttf";

        scTheme.Apply(this, fontSize, fontName);

        XPM xpm(breakpoint_xpm);
        RGBAImage bpImage(xpm);

        SendCommand(SCI_SETMARGINWIDTHN, 0, 44);//Calculate correct width
        SendCommand(SCI_SETMARGINTYPEN, 1, SC_MARGIN_SYMBOL);
        SendCommand(SCI_SETMARGINMASKN, 1, ~SC_MASK_FOLDERS); // allow everything except for the folding symbols
        SendCommand(SCI_RGBAIMAGESETSCALE, 100);
        SendCommand(SCI_SETMARGINWIDTHN, 1, bpImage.GetWidth());
        SendCommand(SCI_RGBAIMAGESETWIDTH, (uptr_t)bpImage.GetWidth());
        SendCommand(SCI_RGBAIMAGESETHEIGHT, (uptr_t)bpImage.GetHeight());
        SendCommand(SCI_MARKERDEFINERGBAIMAGE, 0, sptr_t(bpImage.Pixels()));

        /*

           const int imgWidth = 16;
           const int imgHeight = 16;

           SendCommand(SCI_SETMARGINWIDTHN, 1, imgWidth);
           SendCommand(SCI_RGBAIMAGESETWIDTH, imgWidth);
           SendCommand(SCI_RGBAIMAGESETHEIGHT, imgHeight);


           unsigned char* imgData = static_cast<unsigned char*>(malloc(sizeof(unsigned char) * 4 * imgWidth * imgHeight));
           for (int y = 0; y < imgHeight; ++y)
           {
            unsigned char* rowData = &imgData[imgWidth * 4 * y];
            for (int x = 0; x < imgWidth; ++x, rowData += 4)
            {
                rowData[0] = 0;
                rowData[1] = 255;
                rowData[2] = 0;
                rowData[3] = 255;
            }
           }

           SendCommand(SCI_MARKERDEFINERGBAIMAGE, 0, sptr_t(imgData));
           free(imgData);*/

        //SCI_MARKERDEFINEPIXMAP
        SendCommand(SCI_MARKERDEFINE, 0, SC_MARK_RGBAIMAGE);

        SetFocusState(true);
        CaretSetPeriod(0);

           size_t textSize = 0;
           const char* text = static_cast<const char*>(File_loadToMemory("examples/fake_6502/fake6502_main.c", &textSize, 0));
           assert(text);

           SendCommand(SCI_ADDTEXT, textSize,
                    reinterpret_cast<sptr_t>(static_cast<const char*>(text)));

           free((void*)text);

        SendCommand(SCI_MARKERADD, 0, 0);
        SendCommand(SCI_MARKERADD, 1, 0);
        SendCommand(SCI_MARKERADD, 2, 0);

        // Need to do this after setting the text
        //SendCommand(SCI_SETREADONLY, 1);
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void Resize(int x, int y, int width, int height)
    {
        m_width = width;
        m_height = height;

        (void)x;
        (void)y;

        wMain.SetPosition(PRectangle::FromInts(0, 0, m_width, m_height));

        // GW-TODO: Likely need to adjust a member var on wMain and make
        // GetClientRectangle return that value.

        //float w1 = m_width  - 80.0f;
        //float h1 = m_height - 80.0f;/*80=30+20+30*/

        //SetSize(w1 * 0.7f, h1 * 0.7f);
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    virtual void SetVerticalScrollPos() override
    {
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    virtual void SetHorizontalScrollPos() override
    {
        xOffset = 0;
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    bool ModifyScrollBars(int nMax, int nPage)
    {
        (void)nMax;
        (void)nPage;
        return false;
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    virtual void CreateCallTipWindow(PRectangle rc) override
    {
        (void)rc;
        if (!ct.wCallTip.Created())
        {
            //ct.wCallTip = new CallTip(stc, &ct, this);
            ct.wCallTip = AllocateWindowImpl();
            ct.wDraw = ct.wCallTip;
        }
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    virtual void AddToPopUp(const char* label, int cmd = 0, bool enabled = true) override
    {
        (void)label;
        (void)cmd;
        (void)enabled;

    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void ClaimSelection()
    {
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void Copy()
    {
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void Paste()
    {
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void NotifyChange()
    {
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void NotifyParent(SCNotification scn)
    {
        (void)scn;
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void CopyToClipboard(const SelectionText& selectedText)
    {
        (void)selectedText;
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void SetMouseCapture(bool on)
    {
        (void)on;
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    bool HaveMouseCapture()
    {
        return false;
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    sptr_t DefWndProc(unsigned int iMessage, uptr_t wParam, sptr_t lParam)
    {
        (void)iMessage;
        (void)wParam;
        (void)lParam;
        // GW: These are commands\events not handled by Scintilla Editor
        // Do not call into WndProc or it'll be recursive overflow.
        return 0;//WndProc(iMessage, wParam, lParam);
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    sptr_t SendCommand(unsigned int iMessage, uptr_t wParam = 0, sptr_t lParam = 0)
    {
		// Handle our messages first and the fallback on default path

    	switch (iMessage)
		{
			case SCN_TOGGLE_BREAKPOINT:
			{
				ToggleBreakpoint();
				return 0;
			}

			case SCN_GETCURRENT_LINE:
			{
				Point caretPosition = PointMainCaret();
				return (sptr_t)LineFromLocation(caretPosition);
			}
		}

        return WndProc(iMessage, wParam, lParam);
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    std::vector<unsigned int> m_breakpointLines;

    // Interface sent to external code

    ImScEditor interface;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

intptr_t ImScEditor::SendCommand(unsigned int message, uintptr_t p0, intptr_t p1)
{
    ScEditor* editor = (ScEditor*)privateData;
    return (intptr_t)editor->SendCommand(message, (uptr_t)p0, (sptr_t)p1);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ImScEditor::Update()
{
    ScEditor* editor = (ScEditor*)privateData;
    ScEditor_tick(editor);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ImScEditor::Draw()
{
    ScEditor* editor = (ScEditor*)privateData;
    ScEditor_render(editor);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ImScEditor::HandleInput()
{
    ScEditor* editor = (ScEditor*)privateData;
    if (editor)
    	editor->HandleInput();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ImScEditor::ScrollTo(int line, bool moveThumb)
{
    ScEditor* editor = (ScEditor*)privateData;
    editor->ScrollTo(line, moveThumb);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ScEclipseTheme::ScEclipseTheme()
    : m_themeId(0)
{
    //m_themeName[0] = '\0';
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ScEclipseTheme::~ScEclipseTheme()
{
}

#ifdef _WIN32
extern "C" __declspec(dllimport) void __stdcall OutputDebugStringA(const char* _str);
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool ScEclipseTheme::Load(const char* themeName)
{
    using namespace tinyxml2;

    XMLDocument xmlDocument;
    XMLError result = xmlDocument.LoadFile(themeName);
    if (result != XML_SUCCESS)
        return false;

    XMLElement* rootElement = xmlDocument.RootElement();
    if (!rootElement)
        return false;

    XMLElement* colorTheme = xmlDocument.FirstChildElement("colorTheme");

    const char* name = colorTheme->Attribute("name");
    if (name == nullptr)
        return false;

    //strcpy(m_themeName, name);

    m_themeId = (unsigned int)colorTheme->IntAttribute("id");
    if (m_themeId == 0)
        return false;

    bool success = true;

    success &= LoadColour(m_searchResultIndication, "searchResultIndication", colorTheme);
    success &= LoadColour(m_filteredSearchResultIndication, "filteredSearchResultIndication", colorTheme);
    success &= LoadColour(m_occurrenceIndication, "occurrenceIndication", colorTheme);
    success &= LoadColour(m_writeOccurrenceIndication, "writeOccurrenceIndication", colorTheme);
    success &= LoadColour(m_findScope, "findScope", colorTheme);
    success &= LoadColour(m_sourceHoverBackground, "sourceHoverBackground", colorTheme);
    success &= LoadColour(m_singleLineComment, "singleLineComment", colorTheme);
    success &= LoadColour(m_multiLineComment, "multiLineComment", colorTheme);
    success &= LoadColour(m_commentTaskTag, "commentTaskTag", colorTheme);
    success &= LoadColour(m_javadoc, "javadoc", colorTheme);
    success &= LoadColour(m_javadocLink, "javadocLink", colorTheme);
    success &= LoadColour(m_javadocTag, "javadocTag", colorTheme);
    success &= LoadColour(m_javadocKeyword, "javadocKeyword", colorTheme);
    success &= LoadColour(m_class, "class", colorTheme);
    success &= LoadColour(m_interface, "interface", colorTheme);
    success &= LoadColour(m_method, "method", colorTheme);
    success &= LoadColour(m_methodDeclaration, "methodDeclaration", colorTheme);
    success &= LoadColour(m_bracket, "bracket", colorTheme);
    success &= LoadColour(m_number, "number", colorTheme);
    success &= LoadColour(m_string, "string", colorTheme);
    success &= LoadColour(m_operator, "operator", colorTheme);
    success &= LoadColour(m_keyword, "keyword", colorTheme);
    success &= LoadColour(m_annotation, "annotation", colorTheme);
    success &= LoadColour(m_staticMethod, "staticMethod", colorTheme);
    success &= LoadColour(m_localVariable, "localVariable", colorTheme);
    success &= LoadColour(m_localVariableDeclaration, "localVariableDeclaration", colorTheme);
    success &= LoadColour(m_field, "field", colorTheme);
    success &= LoadColour(m_staticField, "staticField", colorTheme);
    success &= LoadColour(m_staticFinalField, "staticFinalField", colorTheme);
    success &= LoadColour(m_deprecatedMember, "deprecatedMember", colorTheme);
    success &= LoadColour(m_background, "background", colorTheme);
    success &= LoadColour(m_currentLine, "currentLine", colorTheme);
    success &= LoadColour(m_foreground, "foreground", colorTheme);
    success &= LoadColour(m_lineNumber, "lineNumber", colorTheme);
    success &= LoadColour(m_selectionBackground, "selectionBackground", colorTheme);
    success &= LoadColour(m_selectionForeground, "selectionForeground", colorTheme);

    return success;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool ScEclipseTheme::LoadColour(ColourDesired& colour, const char* name, tinyxml2::XMLElement* element)
{
    using namespace tinyxml2;

    XMLElement* colourElement = element->FirstChildElement(name);
    if (colourElement == nullptr)
        return false;

    const char* colourHtml = colourElement->Attribute("color");
    if (colourHtml == nullptr)
        return false;

    htmlToColour(colour, colourHtml);
    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ScEclipseTheme::Apply(struct ScEditor* editor, int fontSize, const char* fontName)
{
    // Set up the global default style. These attributes are used wherever no explicit choices are made.
    editor->SetAStyle(STYLE_DEFAULT, m_foreground, m_background, fontSize, fontName);
    editor->SendCommand(SCI_STYLECLEARALL); // Copies global style to all others
    editor->SetAStyle(STYLE_INDENTGUIDE, 0xFFC0C0C0, m_background, fontSize, fontName);
    editor->SetAStyle(STYLE_BRACELIGHT, m_bracket, m_background, fontSize, fontName);
    editor->SetAStyle(STYLE_BRACEBAD, m_bracket, m_background, fontSize, fontName);
    editor->SetAStyle(STYLE_LINENUMBER, m_lineNumber, 0xD0333333, fontSize, fontName);

    editor->SetAStyle(SCE_C_DEFAULT, m_foreground, m_background, fontSize, fontName);
    editor->SetAStyle(SCE_C_STRING, m_string, m_background);// GW-TODO: Pick a good color
    editor->SetAStyle(SCE_C_IDENTIFIER, m_method, m_background);
    editor->SetAStyle(SCE_C_CHARACTER, m_string, m_background); // GW-TODO: Pick a good color
    editor->SetAStyle(SCE_C_WORD, m_keyword, m_background);
    editor->SetAStyle(SCE_C_WORD2, m_keyword, m_background);
    editor->SetAStyle(SCE_C_GLOBALCLASS, m_class, m_background);
    editor->SetAStyle(SCE_C_PREPROCESSOR, m_annotation, m_background);
    editor->SetAStyle(SCE_C_NUMBER, m_number, m_background);
    editor->SetAStyle(SCE_C_OPERATOR, m_operator, m_background);
    editor->SetAStyle(SCE_C_COMMENT, m_multiLineComment, m_background);
    editor->SetAStyle(SCE_C_COMMENTLINE, m_singleLineComment, m_background);
    editor->SetAStyle(SCE_C_COMMENTDOC, m_multiLineComment, m_background);

    //SCE_C_COMMENTDOCKEYWORD
    //SCE_C_COMMENTDOCKEYWORDERROR

    // text->StyleSetBold(wxSTC_C_WORD, true);
    // text->StyleSetBold(wxSTC_C_WORD2, true);
    //text->StyleSetBold(wxSTC_C_COMMENTDOCKEYWORD, true);

    editor->SendCommand(SCI_SETSELBACK, 1, m_background.AsLong());
    // editor->SendCommand(SCI_SETSELBACK, 1, 0xD0CC9966);
    editor->SendCommand(SCI_SETCARETFORE, 0xFFFFFFFF, 0);
    editor->SendCommand(SCI_SETCARETLINEVISIBLE, 1);
    editor->SendCommand(SCI_SETCARETLINEBACK, 0xFFFFFFFF);
    editor->SendCommand(SCI_SETCARETLINEBACKALPHA, 0x20);

    editor->SendCommand(SCI_SETUSETABS, 1);
    editor->SendCommand(SCI_SETTABWIDTH, 4);
    editor->SendCommand(SCI_SETINDENTATIONGUIDES, SC_IV_REAL);

    editor->SendCommand(SCI_MARKERSETBACK, 0, 0xFF6A6A6A);
    editor->SendCommand(SCI_MARKERSETFORE, 0, 0xFF0000FF);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ScEditor* ScEditor_create(int width, int height)
{
    ScEditor* ed = new ScEditor;

    ed->interface.userData = 0;
    ed->interface.privateData = ed;

    ed->Initialise();
    ScEditor_resize(ed, 0, 0, width, height);

    return ed;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ScEditor_resize(ScEditor* editor, int x, int y, int width, int height)
{
    (void)x;
    (void)y;

    if (editor)
        editor->Resize(0, 0, width, height);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ScEditor_tick(ScEditor* editor)
{
    if (editor)
        editor->Update();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ScEditor_render(ScEditor* editor)
{
    if (editor)
        editor->Render();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ScEditor_scrollMouse(ScEditor* editor, const PDMouseWheelEvent& wheelEvent)
{
    if (editor)
        editor->HandleMouseWheel(wheelEvent);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ImScEditor* ScEditor_getInterface(ScEditor* editor)
{
    return &editor->interface;
}
