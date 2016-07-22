VSH��� 
u_viewRect   �  attribute highp vec4 a_color0;
attribute highp vec2 a_position;
attribute highp vec2 a_texcoord0;
varying highp vec4 v_color0;
varying highp vec2 v_texcoord0;
uniform highp vec4 u_viewRect;
void main ()
{
  highp vec4 tmpvar_1;
  tmpvar_1.zw = vec2(0.0, 1.0);
  tmpvar_1.x = (((2.0 * a_position.x) / u_viewRect.z) - 1.0);
  tmpvar_1.y = (1.0 - ((2.0 * a_position.y) / u_viewRect.w));
  gl_Position = tmpvar_1;
  v_texcoord0 = a_texcoord0;
  v_color0 = a_color0;
}

 