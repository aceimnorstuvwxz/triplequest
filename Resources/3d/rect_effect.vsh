#ifdef GL_ES
precision mediump float;
#endif

// Inputs
attribute vec4 a_positioin;
attribute vec4 a_color;
attribute vec4 a_expand_config;
attribute vec2 a_radio_config;


// Varyings
#ifdef GL_ES
varying mediump vec4 v_color;
varying mediump vec2 v_radio_config;
varying mediump float v_expand_start;
#endif

uniform float u_time;

void main()
{
    gl_Position = CC_MVPMatrix * a_positioin;
    gl_PointSize = a_expand_config.r - (a_expand_config.r - a_expand_config.g)*smoothstep(a_expand_config.b, a_expand_config.a, u_time);
    v_color = a_color;
    v_radio_config = a_radio_config;
    v_expand_start = a_expand_config.b;
}