#include "ui_sc_editor.h"

#include <stdlib.h>
#include <stddef.h>
#include <algorithm>
#include <map>
#include <vector>
#include <string>

#include "scintilla/include/Platform.h"
#include "scintilla/include/Scintilla.h"
#include "scintilla/include/ILexer.h"
#include "scintilla/include/SciLexer.h"
#include "scintilla/src/lexlib/LexerModule.h"
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

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

const char glslKeyword[] =
"discard struct if else switch case default break goto return for while do continue";

const char glslType[] =
"attribute const in inout out uniform varying invariant "
"centroid flat smooth noperspective layout patch sample "
"subroutine lowp mediump highp precision "
"void float vec2 vec3 vec4 bvec2 bvec3 bvec4 ivec2 ivec3 ivec4 "
"uvec2 uvec3 uvec4 dvec2 dvec3 dvec4 "
"sampler1D sampler2D sampler3D isampler2D isampler1D isampler3D "
"usampler1D usampler2D usampler3D "
"sampler1DShadow sampler2DShadow sampler1DArray sampler2DArray "
"sampler1DArrayShadow sampler2DArrayShadow "
"samplerCube samperCubeShadow samperCubeArrayShadow ";

const char glslBuiltin[] =
"radians degrees sin cos tan asin acos atan sinh "
"cosh tanh asinh acosh atanh pow exp log exp2 "
"log2 sqrt inversesqrt abs sign floor trunc round "
"roundEven ceil fract mod modf min max clamp mix "
"step smoothstep isnan isinf floatBitsToInt floatBitsToUint "
"intBitsToFloat uintBitsToFloat fma frexp ldexp packUnorm2x16 "
"packUnorm4x8 packSnorm4x8 unpackUnorm2x16 unpackUnorm4x8 "
"unpackSnorm4x8 packDouble2x32 unpackDouble2x32 length distance "
"dot cross normalize ftransform faceforward reflect "
"refract matrixCompMult outerProduct transpose determinant "
"inverse lessThan lessThanEqual greaterThan greaterThanEqual "
"equal notEqual any all not uaddCarry usubBorrow "
"umulExtended imulExtended bitfieldExtract bitfildInsert "
"bitfieldReverse bitCount findLSB findMSB textureSize "
"textureQueryLOD texture textureProj textureLod textureOffset "
"texelFetch texelFetchOffset textureProjOffset textureLodOffset "
"textureProjLod textureProjLodOffset textureGrad textureGradOffset "
"textureProjGrad textureProjGradOffset textureGather "
"textureGatherOffset texture1D texture2D texture3D texture1DProj "
"texture2DProj texture3DProj texture1DLod texture2DLod "
"texture3DLod texture1DProjLod texture2DProjLod texture3DProjLod "
"textureCube textureCubeLod shadow1D shadow2D shadow1DProj "
"shadow2DProj shadow1DLod shadow2DLod shadow1DProjLod "
"shadow2DProjLod dFdx dFdy fwidth interpolateAtCentroid "
"interpolateAtSample interpolateAtOffset noise1 noise2 noise3 "
"noise4 EmitStreamVertex EndStreamPrimitive EmitVertex "
"EndPrimitive barrier "
"gl_VertexID gl_InstanceID gl_Position gl_PointSize "
"gl_ClipDistance gl_PrimitiveIDIn gl_InvocationID gl_PrimitiveID "
"gl_Layer gl_PatchVerticesIn gl_TessLevelOuter gl_TessLevelInner "
"gl_TessCoord gl_FragCoord gl_FrontFacing gl_PointCoord "
"gl_SampleID gl_SamplePosition gl_FragColor gl_FragData "
"gl_FragDepth gl_SampleMask gl_ClipVertex gl_FrontColor "
"gl_BackColor gl_FrontSecondaryColor gl_BackSecondaryColor "
"gl_TexCoord gl_FogFragCoord gl_Color gl_SecondaryColor "
"gl_Normal gl_Vertex gl_MultiTexCoord0 gl_MultiTexCoord1 "
"gl_MultiTexCoord2 gl_MultiTexCoord3 gl_MultiTexCoord4 "
"gl_MultiTexCoord5 gl_MultiTexCoord6 gl_MultiTexCoord7 gl_FogCoord "
"gl_MaxVertexAttribs gl_MaxVertexUniformComponents gl_MaxVaryingFloats "
"gl_MaxVaryingComponents gl_MaxVertexOutputComponents "
"gl_MaxGeometryInputComponents gl_MaxGeometryOutputComponents "
"gl_MaxFragmentInputComponents gl_MaxVertexTextureImageUnits "
"gl_MaxCombinedTextureImageUnits gl_MaxTextureImageUnits "
"gl_MaxFragmentUniformComponents gl_MaxDrawBuffers gl_MaxClipDistances "
"gl_MaxGeometryTextureImageUnits gl_MaxGeometryOutputVertices "
"gl_MaxGeometryTotalOutputComponents gl_MaxGeometryUniformComponents "
"gl_MaxGeometryVaryingComponents gl_MaxTessControlInputComponents "
"gl_MaxTessControlOutputComponents gl_MaxTessControlTextureImageUnits "
"gl_MaxTessControlUniformComponents "
"gl_MaxTessControlTotalOutputComponents "
"gl_MaxTessEvaluationInputComponents gl_MaxTessEvaluationOutputComponents "
"gl_MaxTessEvaluationTextureImageUnits "
"gl_MaxTessEvaluationUniformComponents gl_MaxTessPatchComponents "
"gl_MaxPatchVertices gl_MaxTessGenLevel gl_MaxTextureUnits "
"gl_MaxTextureCoords gl_MaxClipPlanes "
"gl_DepthRange gl_ModelViewMatrix gl_ProjectionMatrix "
"gl_ModelViewProjectionMatrix gl_TextureMatrix gl_NormalMatrix "
"gl_ModelViewMatrixInverse gl_ProjectionMatrixInverse "
"gl_ModelViewProjectionMatrixInverse gl_TextureMatrixInverse "
"gl_ModelViewMatrixTranspose gl_ProjectionMatrixTranspose "
"gl_ModelViewProjectionMatrixTranspose gl_TextureMatrixTranspose "
"gl_ModelViewMatrixInverseTranspose gl_ProjectionMatrixInverseTranspose "
"gl_ModelViewProjectionMatrixInverseTranspose "
"gl_TextureMatrixInverseTranspose gl_NormalScale gl_ClipPlane "
"gl_Point gl_FrontMaterial gl_BackMaterial gl_LightSource "
"gl_LightModel gl_FrontLightModelProduct gl_BackLightModelProduct "
"gl_FrontLightProduct gl_BackLightProduct gl_TextureEnvColor "
"gl_EyePlaneS gl_EyePlaneT gl_EyePlaneR gl_EyePlaneQ "
"gl_ObjectPlaneS gl_ObjectPlaneT gl_ObjectPlaneR gl_ObjectPlaneQ "
"gl_Fog";

class LexState : public LexInterface
{
    const LexerModule* lexCurrent;

public:
    int lexLanguage;
    
    LexState(Document *pdoc_)
    : LexInterface(pdoc_)
    {
        lexCurrent = 0;
        performingStyle = false;
        lexLanguage = SCLEX_CONTAINER;
    }
    
    ~LexState()
    {
        if (instance)
        {
            instance->Release();
            instance = 0;
        }
    }
    
    void SetLexerModule(const LexerModule *lex)
    {
        if (lex != lexCurrent)
        {
            if (instance)
            {
                instance->Release();
                instance = 0;
            }
            
            lexCurrent = lex;
            
            if (lexCurrent)
                instance = lexCurrent->Create();
            
            pdoc->LexerChanged();
        }
    }
    
    void SetLexer(uptr_t wParam)
    {
        lexLanguage = int(wParam);
        if (lexLanguage == SCLEX_CONTAINER)
            SetLexerModule(0);
        else
        {
            const LexerModule *lex = Catalogue::Find(lexLanguage);
            if (!lex)
                lex = Catalogue::Find(SCLEX_NULL);
            SetLexerModule(lex);
        }
    }
    
    void SetLexerLanguage(const char *languageName)
    {
        const LexerModule *lex = Catalogue::Find(languageName);
        if (!lex)
            lex = Catalogue::Find(SCLEX_NULL);
        if (lex)
            lexLanguage = lex->GetLanguage();
        SetLexerModule(lex);
    }
    
    void SetWordList(int n, const char *wl)
    {
        if (instance)
        {
            int firstModification = instance->WordListSet(n, wl);
            if (firstModification >= 0)
                pdoc->ModifiedAt(firstModification);
        }
    }
    
    void PropSet(const char *key, const char *val)
    {
        if (instance)
        {
            int firstModification = instance->PropertySet(key, val);
            if (firstModification >= 0)
                pdoc->ModifiedAt(firstModification);
        }
    }
};

const size_t NB_FOLDER_STATE = 7;
const size_t FOLDER_TYPE = 0;
const uptr_t markersArray[][NB_FOLDER_STATE] =
{
    {SC_MARKNUM_FOLDEROPEN, SC_MARKNUM_FOLDER, SC_MARKNUM_FOLDERSUB, SC_MARKNUM_FOLDERTAIL, SC_MARKNUM_FOLDEREND,        SC_MARKNUM_FOLDEROPENMID,     SC_MARKNUM_FOLDERMIDTAIL},
    {SC_MARK_MINUS,         SC_MARK_PLUS,      SC_MARK_EMPTY,        SC_MARK_EMPTY,         SC_MARK_EMPTY,               SC_MARK_EMPTY,                SC_MARK_EMPTY},
    {SC_MARK_ARROWDOWN,     SC_MARK_ARROW,     SC_MARK_EMPTY,        SC_MARK_EMPTY,         SC_MARK_EMPTY,               SC_MARK_EMPTY,                SC_MARK_EMPTY},
    {SC_MARK_CIRCLEMINUS,   SC_MARK_CIRCLEPLUS,SC_MARK_VLINE,        SC_MARK_LCORNERCURVE,  SC_MARK_CIRCLEPLUSCONNECTED, SC_MARK_CIRCLEMINUSCONNECTED, SC_MARK_TCORNERCURVE},
    {SC_MARK_BOXMINUS,      SC_MARK_BOXPLUS,   SC_MARK_VLINE,        SC_MARK_LCORNER,       SC_MARK_BOXPLUSCONNECTED,    SC_MARK_BOXMINUSCONNECTED,    SC_MARK_TCORNER}
};

const ColourDesired black(0,0,0);
const ColourDesired white(0xff,0xff,0xff);

struct ScEditor : public Editor
{
private:
    LexState* m_lexer;
    int m_width;
    int m_height;
    
public:
    
    void Update()
    {
        Tick();
    }
    
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
    
    void SetAStyle(int style, ColourDesired fore, ColourDesired back=0xFFFFFFFF, int size=-1, const char *face=0)
    {
        SendCommand(SCI_STYLESETFORE, uptr_t(style), fore.AsLong());
        SendCommand(SCI_STYLESETBACK, uptr_t(style), back.AsLong());
        if (size >= 1)
            SendCommand(SCI_STYLESETSIZE, uptr_t(style), size);
        if (face)
            SendCommand(SCI_STYLESETFONT, uptr_t(style), reinterpret_cast<sptr_t>(face));
    }
    
	void Initialise()
	{
        wMain = WindowID(1);
        wMargin = WindowID(2);
        
        m_lexer = new LexState(pdoc);
        
        SendCommand(SCI_SETSTYLEBITS, 7);
        
        m_lexer->SetLexer(SCLEX_CPP);
        m_lexer->SetWordList(0, glslKeyword);
        m_lexer->SetWordList(1, glslType);
        m_lexer->SetWordList(4, glslBuiltin);
        m_lexer->PropSet("fold", "1");
        
        int fontSize = 24;
        
        const char* fontName = "data/font/source_code_pro/SourceCodePro-Medium.ttf";
        
        // Set up the global default style. These attributes are used wherever no explicit choices are made.
        SetAStyle(STYLE_DEFAULT,     0xFFFFFFFF, 0xD0000000, fontSize, fontName);
        SendCommand(SCI_STYLECLEARALL);	// Copies global style to all others
        SetAStyle(STYLE_INDENTGUIDE, 0xFFC0C0C0, 0xD0000000, fontSize, fontName);
        SetAStyle(STYLE_BRACELIGHT,  0xFF00FF00, 0xD0000000, fontSize, fontName);
        SetAStyle(STYLE_BRACEBAD,    0xFF0000FF, 0xD0000000, fontSize, fontName);
        SetAStyle(STYLE_LINENUMBER,  0xFFC0C0C0, 0xD0333333, fontSize, fontName);
        SendCommand(SCI_SETFOLDMARGINCOLOUR,   1, 0xD01A1A1A);
        SendCommand(SCI_SETFOLDMARGINHICOLOUR, 1, 0xD01A1A1A);
        SendCommand(SCI_SETSELBACK,            1, 0xD0CC9966);
        SendCommand(SCI_SETCARETFORE,          0xFFFFFFFF, 0);
        SendCommand(SCI_SETCARETLINEVISIBLE,   1);
        SendCommand(SCI_SETCARETLINEBACK,      0xFFFFFFFF);
        SendCommand(SCI_SETCARETLINEBACKALPHA, 0x20);
        
        SendCommand(SCI_SETMARGINWIDTHN, 0, 44);//Calculate correct width
        SendCommand(SCI_SETMARGINWIDTHN, 1, 20);//Calculate correct width
        SendCommand(SCI_SETMARGINMASKN, 1, SC_MASK_FOLDERS);//Calculate correct width
        
        for (unsigned int i = 0 ; i < NB_FOLDER_STATE ; i++)
        {
            SendCommand(SCI_MARKERDEFINE, markersArray[FOLDER_TYPE][i], sptr_t(markersArray[4][i]));
            SendCommand(SCI_MARKERSETBACK, markersArray[FOLDER_TYPE][i], 0xFF6A6A6A);
            SendCommand(SCI_MARKERSETFORE, markersArray[FOLDER_TYPE][i], 0xFF333333);
        }
        
        SendCommand(SCI_SETUSETABS, 1);
        SendCommand(SCI_SETTABWIDTH, 4);
        SendCommand(SCI_SETINDENTATIONGUIDES, SC_IV_REAL);
        
        SetAStyle(SCE_C_DEFAULT,      0xFFFFFFFF, 0xD0000000, fontSize, fontName);
        SetAStyle(SCE_C_WORD,         0xFF0066FF, 0xD0000000);
        SetAStyle(SCE_C_WORD2,        0xFFFFFF00, 0xD0000000);
        //WTF??? SetAStyle(SCE_C_GLOBALCLASS, 0xFF0000FF, 0xFF000000);
        SetAStyle(SCE_C_PREPROCESSOR, 0xFFC0C0C0, 0xD0000000);
        SetAStyle(SCE_C_NUMBER,       0xFF0080FF, 0xD0000000);
        SetAStyle(SCE_C_OPERATOR,     0xFF00CCFF, 0xD0000000);
        SetAStyle(SCE_C_COMMENT,      0xFF00FF00, 0xD0000000);
        SetAStyle(SCE_C_COMMENTLINE,  0xFF00FF00, 0xD0000000);
        
        const char* text = "Hello World!";
        
        SendCommand(SCI_ADDTEXT, strlen(text),
                   reinterpret_cast<sptr_t>(static_cast<const char *>(text)));
	}
    
    void Resize(int width, int height)
    {
        m_width  = width;
        m_height = height;
        
        // GW-TODO: Likely need to adjust a member var on wMain and make
        // GetClientRectangle return that value.
        
        //float w1 = m_width  - 80.0f;
        //float h1 = m_height - 80.0f;/*80=30+20+30*/
        
        //SetSize(w1 * 0.7f, h1 * 0.7f);
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
    
    sptr_t SendCommand(unsigned int iMessage, uptr_t wParam = 0, sptr_t lParam = 0)
    {
        return WndProc(iMessage, wParam, lParam);
    }
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ScEditor* ScEditor_create(int width, int height)
{
	ScEditor* ed = new ScEditor;
    
    ed->Initialise();
    ScEditor_resize(ed, width, height);

	return ed;
}

void ScEditor_resize(ScEditor* editor, int width, int height)
{
    if (editor)
        editor->Resize(width, height);
}

void ScEditor_tick(ScEditor* editor)
{
    if (editor)
        editor->Update();
}

void ScEditor_render(ScEditor* editor)
{
    if (editor)
    {
        //editor->BraceMatch();
        editor->Render();
    }
}
