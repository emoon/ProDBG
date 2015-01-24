$input v_color0, v_texcoord0

#include "../common.sh"

SAMPLER2D(s_tex, 0);

void main()
{
	vec4 t = texture2D(s_tex, v_texcoord0);
	vec4 texel = vec4(t.r, t.r, t.r, t.r);
	gl_FragColor = texel * v_color0; 
}

