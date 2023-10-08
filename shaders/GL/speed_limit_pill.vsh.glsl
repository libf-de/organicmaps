attribute vec2 a_position;
attribute vec3 a_color;
attribute vec3 a_outlineColor;
attribute float a_radius;
attribute float a_outlineWidthRatio;
attribute float a_halfPillHeight;
attribute float a_bottomOpacity;

uniform mat4 u_modelView;
uniform mat4 u_projection;

varying vec2 v_position;
varying vec3 v_color;
varying vec3 v_outlineColor;
varying float v_outlineWidthRatio;
varying float v_halfPillHeight;
varying float v_bottomOpacity;

void main()
{
  v_position = a_position;
  v_color = a_color;
  v_outlineColor = a_outlineColor;
  v_outlineWidthRatio = a_outlineWidthRatio;
  v_halfPillHeight = a_halfPillHeight;
  v_bottomOpacity = a_bottomOpacity;

  gl_Position = vec4(a_position * a_radius, 0, 1) * u_modelView * u_projection;
#ifdef VULKAN
  gl_Position.y = -gl_Position.y;
  gl_Position.z = (gl_Position.z  + gl_Position.w) * 0.5;
#endif
}
