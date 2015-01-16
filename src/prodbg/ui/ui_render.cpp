#include <bgfx.h>
#include "core/log.h"
#include "core/file.h"
#include "core/core.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bgfx::UniformHandle s_tex;
static bgfx::UniformHandle u_viewSize;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct ProgramAttribs
{
	bgfx::Attrib::Enum attrib;
	uint8_t num;
	bgfx::AttribType::Enum type;
	bool norm;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct ProgramInfo
{
	ProgramAttribs* attribs;
	const char* vsName;
	const char* fsName;
	bgfx::VertexDecl vertexDecl;
	bgfx::ProgramHandle handle;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

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

static ProgramAttribs posTexColorAttribs[] =
{
	{ bgfx::Attrib::Position, 2, bgfx::AttribType::Float, false },
	{ bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float, false },
	{ bgfx::Attrib::Color0, 4, bgfx::AttribType::Uint8, true },
	{ bgfx::Attrib::Count, 0, bgfx::AttribType::Uint8 },
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static ProgramInfo s_programs[] =
{
	{
		(ProgramAttribs*)&posTexColorAttribs,
		OBJECT_DIR "/_generated/data/shaders/imgui/vs_imgui.vs",
		OBJECT_DIR "/_generated/data/shaders/imgui/fs_imgui.fs",
	},
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

enum
{
	Program_PosTexColor,
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool UIRender_init()
{
    u_viewSize = bgfx::createUniform("viewSize", bgfx::UniformType::Uniform2fv);
    s_tex = bgfx::createUniform("s_tex", bgfx::UniformType::Uniform1i);

	for (int i = 0; i < (int)sizeof_array(s_programs); ++i)
	{
		ProgramInfo* program = &s_programs[i];

		program->handle = loadProgram(program->vsName, program->fsName);

		if (!isValid(program->handle))
			return false;

		const ProgramAttribs* attribs = program->attribs;
		bgfx::VertexDecl& decl = program->vertexDecl;

		decl.begin();

		while (attribs->attrib != bgfx::Attrib::Count)
		{
			decl = decl.add(attribs->attrib, attribs->num, attribs->type, attribs->norm);
			attribs++;
		}

		decl.end();
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void UIRender_allocPosTexColorTb(bgfx::TransientVertexBuffer* buffer, int count)
{
	bgfx::allocTransientVertexBuffer(buffer, count, s_programs[Program_PosTexColor].vertexDecl);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void UIRender_posTexColor(
	bgfx::TransientVertexBuffer* vertexBuffer, int offset, int count, bgfx::TextureHandle texHandle, float width, float height)
{
	// TODO: We likely don't need this one.
	float viewSize[2] = { width, height };

	bgfx::setTexture(0, s_tex, texHandle);
	bgfx::setVertexBuffer(vertexBuffer, offset, count);
	bgfx::setProgram(s_programs[Program_PosTexColor].handle);
	bgfx::setUniform(u_viewSize, viewSize);
	bgfx::submit(0);
}





