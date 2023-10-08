varying vec2 v_position;
varying vec3 v_color;
varying vec3 v_outlineColor;
varying float v_outlineWidthRatio;
varying float v_halfPillHeight;
varying float v_bottomOpacity;

#ifdef SAMSUNG_GOOGLE_NEXUS
uniform sampler2D u_colorTex;
#endif


void main()
{
  float R = 1.0; // Circle radius
  float R2 = R - v_outlineWidthRatio; // Inner radius for outline

  float denormHalfPillHeight = v_halfPillHeight - 1.0;

  // Default to transparent
  //gl_FragColor = vec4(0.0, 0.0, 0.0, 0.0);
  vec4 resColor = vec4(0.0, 0.0, 0.0, 0.0);

  // Define top center (negative y)
  vec2 topCenter = vec2(0.0, -denormHalfPillHeight);
  float topDist = length(v_position - topCenter);

  // Define bottom center (positive y)
  vec2 bottomCenter = vec2(0.0, denormHalfPillHeight);
  float bottomDist = length(v_position - bottomCenter);

  // First check if we're inside the pill shape
  bool insidePill = false;

  if ((v_position.y <= -denormHalfPillHeight && topDist <= R) ||               // Top half-circle
  (v_position.y >= denormHalfPillHeight && bottomDist <= R) ||             // Bottom half-circle
  (v_position.y > -denormHalfPillHeight && v_position.y < denormHalfPillHeight && abs(v_position.x) <= R)) { // Rectangle
    insidePill = true;
  }

  // If we're inside the pill, set the fill color
  if (insidePill) {
    //gl_FragColor = vec4(v_color, v_bottomOpacity);
    resColor = vec4(v_color, v_bottomOpacity);
  }

  // Check if we should draw the top circle outline (at negative y)
  // This checks if we're in the outline region of the circle at the TOP of the pill
  if(topDist <= R) {
    if (topDist >= R2) {
      float sm = smoothstep(R, R-0.01, topDist);
      float sm2 = smoothstep(R2, R2+0.01, topDist);
      float alpha = sm * sm2;
      //gl_FragColor = vec4(v_outlineColor, alpha);
      resColor = vec4(v_outlineColor, alpha);
    } else {
      //gl_FragColor = vec4(v_color, 1.0);
      resColor = vec4(v_color, 1.0);
    }
  }

  gl_FragColor = samsungGoogleNexusWorkaround(resColor);
}
