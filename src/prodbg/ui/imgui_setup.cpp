#include "imgui_setup.h"
#include "imgui/imgui.h"
#include "stb_image.h"
#include "core/core.h"
#include "core/file.h"
#include "core/log.h"
#include <stdio.h>
#include <pd_keys.h>
#include <bgfx.h>
#include <bx/fpumath.h>

static bgfx::VertexDecl s_vertexDecl;
static bgfx::ProgramHandle s_imguiProgram;
static bgfx::TextureHandle s_textureId;
static bgfx::UniformHandle s_tex;

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

struct PosColorVertex
{
	float m_x;
	float m_y;
	float m_u;
	float m_v;
	uint32_t m_abgr;

	static void init()
	{
		ms_decl
			.begin()
			.add(bgfx::Attrib::Position, 2, bgfx::AttribType::Float)
			.add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float)
			.add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Uint8, true)
			.end();
	}

	static bgfx::VertexDecl ms_decl;
};

bgfx::VertexDecl PosColorVertex::ms_decl;

#if 0

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

#endif

//static bgfx::VertexBufferHandle vbh;
//static bgfx::IndexBufferHandle ibh;
static bgfx::UniformHandle u_viewSize;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void imguiDummyRender(ImDrawList** const cmd_lists, int cmd_lists_count)
{
    (void)cmd_lists;
    (void)cmd_lists_count;

	float viewSize[2];

	const float width = ImGui::GetIO().DisplaySize.x;
	const float height = ImGui::GetIO().DisplaySize.y;

	viewSize[0] = width;
	viewSize[1] = height;

	//printf("cmd_list_count %d\n", cmd_lists_count);
	
	float ortho[16];
	bx::mtxOrtho(ortho, 0.0f, viewSize[0], viewSize[1], 0.0f, -1.0f, 1.0f);

	bgfx::setViewTransform(0, NULL, ortho);

	// Render command lists
	
	for (int n = 0; n < cmd_lists_count; n++)
	{
		bgfx::TransientVertexBuffer tvb;

		uint32_t vtx_size = 0;

		const ImDrawList* cmd_list = cmd_lists[n];
		const ImDrawVert* vtx_buffer = cmd_list->vtx_buffer.begin();
		(void)vtx_buffer;
		
		const ImDrawCmd* pcmd_end_t = cmd_list->commands.end();

		for (const ImDrawCmd* pcmd = cmd_list->commands.begin(); pcmd != pcmd_end_t; pcmd++)
			vtx_size += (uint32_t)pcmd->vtx_count;

		bgfx::allocTransientVertexBuffer(&tvb, vtx_size, s_vertexDecl);

		ImDrawVert* verts = (ImDrawVert*)tvb.data;

		// temporary only copy the positos

		//printf("coords----------------------------------------\n");

		for (uint32_t i = 0; i < vtx_size; ++i)
		{
			uint32_t c = vtx_buffer[i].col; 
			verts->pos = vtx_buffer[i].pos;
			verts->uv = vtx_buffer[i].uv;
			verts->col = c; //(c & 0xff000000) | 0xff0000;
			//printf("0x%08x\n", verts->col); 
			verts++;
		}

		uint32_t vtx_offset = 0;
		const ImDrawCmd* pcmd_end = cmd_list->commands.end();
		for (const ImDrawCmd* pcmd = cmd_list->commands.begin(); pcmd != pcmd_end; pcmd++)
		{
			bgfx::setScissor((uint16_t)pcmd->clip_rect.x, 
						     (uint16_t)(height - pcmd->clip_rect.w), 
						     (uint16_t)(pcmd->clip_rect.z - pcmd->clip_rect.x), 
						     (uint16_t)(pcmd->clip_rect.w - pcmd->clip_rect.y));
			
			bgfx::setState(0
							| BGFX_STATE_RGB_WRITE
							| BGFX_STATE_ALPHA_WRITE
							| BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_SRC_ALPHA, BGFX_STATE_BLEND_INV_SRC_ALPHA)
							| BGFX_STATE_MSAA);
			bgfx::setTexture(0, s_tex, s_textureId);
			bgfx::setVertexBuffer(&tvb, vtx_offset, pcmd->vtx_count);
			bgfx::setProgram(s_imguiProgram);
			bgfx::setUniform(u_viewSize, viewSize);
			bgfx::submit(0);

			vtx_offset += pcmd->vtx_count;
		}
	}


	//bgfx::TransientIndexBuffer tib;

	//bgfx::allocTransientVertexBuffer(&tvb, 3, s_vertexDecl);
	//bgfx::allocTransientIndexBuffer(&tib, 3);

	//float* verts = (float*)tvb.data;
	//uint16_t* ib = (uint16_t*)tib.data;

	/*
	{-1.0f,  1.0f,  },
	{ 1.0f,  1.0f,  },
	{-1.0f, -1.0f,  },
	*/

	/*
	verts[0] = 0.0f; 
	verts[1] = 0.0f;

	verts[2] = 401.0f;
	verts[3] = 401.0f;

	verts[4] = 400.0f; 
	verts[5] = 0.0f; 
	*/

	//ib[0] = 0;
	//ib[1] = 1;
	//ib[2] = 2;


	//bgfx::setState(BGFX_STATE_DEPTH_TEST_NEVER);

	//bgfx::setProgram(s_imguiProgram);

	//bgfx::setVertexBuffer(&tvb);
	//bgfx::setIndexBuffer(&tib);

	//bgfx::submit(0);
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void IMGUI_setup(int width, int height)
{
    ImGuiIO& io = ImGui::GetIO();
    io.DisplaySize = ImVec2((float)width, (float)height);
    io.DeltaTime = 1.0f / 60.0f;
    io.PixelCenterOffset = 0.5f;

    printf("imgui setup!\n");

	s_imguiProgram = loadProgram(OBJECT_DIR "/_generated/data/shaders/imgui/vs_imgui.vs", 
								 OBJECT_DIR "/_generated/data/shaders/imgui/fs_imgui.fs");
	s_vertexDecl 
		.begin()
		.add(bgfx::Attrib::Position, 2, bgfx::AttribType::Float)
		.add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float)
		.add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Uint8, true)
		.end();

	u_viewSize = bgfx::createUniform("viewSize", bgfx::UniformType::Uniform2fv);
	s_tex = bgfx::createUniform("s_tex", bgfx::UniformType::Uniform1i);

	const void* png_data;
	unsigned int png_size;

	ImGui::GetDefaultFontData(NULL, NULL, &png_data, &png_size);
	int tex_x, tex_y, pitch, tex_comp;

	void* tex_data = stbi_load_from_memory((const unsigned char*)png_data, (int)png_size, &tex_x, &tex_y, &tex_comp, 0);

	pitch = tex_x * 4;

	const bgfx::Memory* mem = bgfx::alloc((uint32_t)(tex_y * pitch));
	memcpy(mem->data, tex_data, size_t(pitch * tex_y));

	bgfx::imageSwizzleBgra8((uint32_t)tex_x, (uint32_t)tex_y, (uint32_t)pitch, tex_data, mem->data);

	s_textureId = bgfx::createTexture2D((uint16_t)tex_x, (uint16_t)tex_y, 1, bgfx::TextureFormat::BGRA8, BGFX_TEXTURE_MIN_POINT | BGFX_TEXTURE_MAG_POINT, mem);

	printf("textureId %d\n", s_textureId.idx);

	stbi_image_free(tex_data);

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

    ImGui::NewFrame();

	ImGui::Button("Test");

    //io.keyDown = keyDown;
    //io.keyMod = keyMod;


	//bgfx::dbgTextClear();
	//bgfx::dbgTextPrintf(0, 1, 0x4f, "bgfx/examples/01-cube");
	//bgfx::submit(0);

	// Some test rendering
	//
	

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
    ImGui::Render();
}



