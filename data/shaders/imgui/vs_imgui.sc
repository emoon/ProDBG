$input a_position

#include "../common.sh"

uniform vec2 viewSize;

void main()
{
	//gl_Position = vec4(2.0 * a_position.x/u_viewSize.x - 1.0, 1.0 - 2.0 * a_position.y/viewSize.y, 0.0, 1.0);
	gl_Position = vec4(2.0 * a_position.x/1280.0 - 1.0, 1.0 - 2.0 * a_position.y/720.0, 0.0, 1.0);

	//gl_Position = mul(u_modelViewProj, vec4(a_position, 1.0) );
	//gl_Position = vec4(a_position * 0.5, 0.0, 1.0);
	//v_color0 = a_color0;
}

