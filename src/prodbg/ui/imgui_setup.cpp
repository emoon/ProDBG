#include "imgui_setup.h"
#include "imgui/imgui.h"
#include "core/core.h"
#include "core/file.h"
#include "core/log.h"
#include <stdio.h>
#include <pd_keys.h>
#include <bgfx.h>
#include <bx/fpumath.h>

static bgfx::VertexDecl s_vertexDecl;
static bgfx::ProgramHandle s_imguiProgram;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// TODO: Move this to some shared code or such?

static const bgfx::Memory* loadShader(const char* filename)
{
	size_t size;
	uint8_t* data = (uint8_t*)File_loadToMemory(filename, &size, 1);

	if (!data)
	{
		log_error("Unable to load shader %s\n", filename)
		return 0;
	}

	bgfx::Memory* mem = (bgfx::Memory*)bgfx::alloc(sizeof(bgfx::Memory));

	// terminate strings

	data[size] = 0;

	mem->data = data;
	mem->size = (uint32_t)size;

	return mem;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bgfx::ProgramHandle loadProgram(const char* vsName, const char* fsName)
{
	bgfx::ProgramHandle ph = { bgfx::invalidHandle };

	const bgfx::Memory* vsShader = loadShader(vsName); 
	const bgfx::Memory* fsShader = loadShader(fsName); 

	if (!vsShader)
		return ph;

	if (!fsShader)
		return ph;

	bgfx::ShaderHandle vsHandle = bgfx::createShader(vsShader);
	bgfx::ShaderHandle fsHandle = bgfx::createShader(fsShader);

	if (!isValid(vsHandle))
	{
		log_error("Unable to load vsShader %s\n", vsName)
		return ph;
	}

	if (!isValid(fsHandle))
	{
		log_error("Unable to load fsShader %s\n", fsName)
		return ph;
	}

	ph = bgfx::createProgram(vsHandle, fsHandle, true);

	if (!isValid(ph))
		log_error("Unable to create shader program for %s %s\n", vsName, fsName);

	return ph;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void imguiDummyRender(ImDrawList** const cmd_lists, int cmd_lists_count)
{
    (void)cmd_lists;
    (void)cmd_lists_count;

}

struct PosColorVertex
{
	float m_x;
	float m_y;
	//float m_z;
	//uint32_t m_abgr;

	static void init()
	{
		ms_decl
			.begin()
			.add(bgfx::Attrib::Position, 2, bgfx::AttribType::Float)
			//.add(bgfx::Attrib::Color0,   4, bgfx::AttribType::Uint8, true)
			.end();
	}

	static bgfx::VertexDecl ms_decl;
};

bgfx::VertexDecl PosColorVertex::ms_decl;

static PosColorVertex s_cubeVertices[8] =
{
	/*
	{-1.0f,  1.0f,  1.0f, 0xff000000 },
	{ 1.0f,  1.0f,  1.0f, 0xff0000ff },
	{-1.0f, -1.0f,  1.0f, 0xff00ff00 },
	{ 1.0f, -1.0f,  1.0f, 0xff00ffff },
	{-1.0f,  1.0f, -1.0f, 0xffff0000 },
	{ 1.0f,  1.0f, -1.0f, 0xffff00ff },
	{-1.0f, -1.0f, -1.0f, 0xffffff00 },
	{ 1.0f, -1.0f, -1.0f, 0xffffffff },
	*/
	{-1.0f,  1.0f,  },
	{ 1.0f,  1.0f,  },
	{-1.0f, -1.0f,  },
	{ 1.0f, -1.0f,  },
};

static const uint16_t s_cubeIndices[36] =
{
	//0, 1, 2, // 0
	//1, 3, 2,
	0, 2, 1, // 2
	//5, 6, 7,
	//0, 2, 4, // 4
	//4, 2, 6,
	//1, 5, 3, // 6
	//5, 7, 3,
	//0, 4, 1, // 8
	//4, 5, 1,
	//2, 3, 6, // 10
	//6, 3, 7,
};

static bgfx::VertexBufferHandle vbh;
static bgfx::IndexBufferHandle ibh;
static bgfx::UniformHandle u_viewSize;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void IMGUI_setup(int width, int height)
{
    ImGuiIO& io = ImGui::GetIO();
    io.DisplaySize = ImVec2((float)width, (float)height);
    io.DeltaTime = 1.0f / 60.0f;
    io.PixelCenterOffset = 0.5f;

	s_imguiProgram = loadProgram(OBJECT_DIR "/_generated/data/shaders/imgui/vs_imgui.vs", 
								 OBJECT_DIR "/_generated/data/shaders/imgui/fs_imgui.fs");
	s_vertexDecl 
		.begin()
		.add(bgfx::Attrib::Position, 2, bgfx::AttribType::Float)
		.end();

	u_viewSize = bgfx::createUniform("viewSize", bgfx::UniformType::Uniform2fv);

	PosColorVertex::init();

	// Create static vertex buffer.
	vbh = bgfx::createVertexBuffer(
		  // Static data can be passed with bgfx::makeRef
		  bgfx::makeRef(s_cubeVertices, sizeof(s_cubeVertices) )
		, PosColorVertex::ms_decl
		);

	// Create static index buffer.
	ibh = bgfx::createIndexBuffer(
		// Static data can be passed with bgfx::makeRef
		bgfx::makeRef(s_cubeIndices, sizeof(s_cubeIndices) )
		);

	// Enable debug text.
	//bgfx::setDebug(BGFX_DEBUG_TEXT);

    io.RenderDrawListsFn = imguiDummyRender;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


void IMGUI_preUpdate(float x, float y, int mouseLmb, int keyDown, int keyMod)
{
	(void)keyDown;
	(void)keyMod;
    ImGuiIO& io = ImGui::GetIO();
    io.DeltaTime = 1.0f / 60.0f;    // TODO: Fix me
    io.MousePos = ImVec2(x, y);
    io.MouseDown[0] = !!mouseLmb;
    //io.keyDown = keyDown;
    //io.keyMod = keyMod;

    //ImGui::NewFrame();

	//bgfx::dbgTextClear();
	//bgfx::dbgTextPrintf(0, 1, 0x4f, "bgfx/examples/01-cube");
	//bgfx::submit(0);

	// Some test rendering
	
	float ortho[16];
	bx::mtxOrtho(ortho, 0.0f, 1280.0f, 720.0f, 0.0f, 0.0f, 1000.0f);

	bgfx::setViewTransform(0, NULL, ortho);

	bgfx::TransientVertexBuffer tvb;
	bgfx::TransientIndexBuffer tib;

	bgfx::allocTransientVertexBuffer(&tvb, 3, s_vertexDecl);
	bgfx::allocTransientIndexBuffer(&tib, 3);

	float* verts = (float*)tvb.data;
	uint16_t* ib = (uint16_t*)tib.data;

	/*
	{-1.0f,  1.0f,  },
	{ 1.0f,  1.0f,  },
	{-1.0f, -1.0f,  },
	*/

	verts[0] = 0.0f; 
	verts[1] = 0.0f;

	verts[2] = 400.0f; 
	verts[3] = 0.0f; 

	verts[4] = 401.0f;
	verts[5] = 401.0f;

	ib[0] = 0;
	ib[1] = 2;
	ib[2] = 1;

	//bgfx::setState(BGFX_STATE_DEPTH_TEST_NEVER);
	bgfx::setState(BGFX_STATE_DEFAULT);

	bgfx::setProgram(s_imguiProgram);

	bgfx::setVertexBuffer(&tvb);
	bgfx::setIndexBuffer(&tib);

	bgfx::submit(0);

	/*
	//float proj[16];
	//bx::mtxProj(proj, 60.0f, float(width)/float(height), 0.1f, 100.0f);
	//bgfx::setViewTransform(0, view, proj);

	float ortho[16];
	bx::mtxOrtho(ortho, 0.0f, 1280.0f, 720.0f, 0.0f, 0.0f, 1000.0f);
	bgfx::setViewTransform(0, NULL, ortho);

	// Set view 0 default viewport.
	//bgfx::setViewRect(0, 0, 0, width, height);

	// Set vertex and fragment shaders.
	bgfx::setProgram(s_imguiProgram);

	// Set vertex and index buffer.
	bgfx::setVertexBuffer(vbh);
	bgfx::setIndexBuffer(ibh);

	// Set render states.
	bgfx::setState(BGFX_STATE_DEFAULT);

	// Submit primitive for rendering to view 0.
	bgfx::submit(0);
	*/
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void IMGUI_setMouse(float x, float y, int mouseLmb)
{
    ImGuiIO& io = ImGui::GetIO();
    io.MousePos = ImVec2(x, y);
    io.MouseDown[0] = !!mouseLmb;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void IMGUI_setKeyDown(int key, int modifier)
{
    ImGuiIO& io = ImGui::GetIO();
    assert(key >= 0 && key <= (int)sizeof_array(io.KeysDown));
    io.KeysDown[key] = true;
    io.KeyCtrl = !!(modifier & PDKEY_CTRL);
    io.KeyShift = !!(modifier & PDKEY_SHIFT);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void IMGUI_setKeyUp(int key, int modifier)
{
    ImGuiIO& io = ImGui::GetIO();
    assert(key >= 0 && key <= (int)sizeof_array(io.KeysDown));
    io.KeysDown[key] = false;
    io.KeyCtrl = !!(modifier & PDKEY_CTRL);
    io.KeyShift = !!(modifier & PDKEY_SHIFT);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void IMGUI_postUpdate()
{
    //ImGui::Render();
}



