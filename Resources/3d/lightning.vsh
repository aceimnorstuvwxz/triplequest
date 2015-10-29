#ifdef GL_ES
precision mediump float;
#endif

// Inputs
attribute vec4 a_positioin;
attribute vec4 a_color;
attribute float a_start_time;
attribute float a_end_time;


// Varyings
#ifdef GL_ES
varying mediump vec4 v_color;
varying mediump float v_start_time;
varying mediump float v_end_time;
#endif


void main()
{
    gl_Position = CC_MVPMatrix * a_positioin;
    v_color = a_color;
    v_start_time = a_start_time;
    v_end_time = a_end_time;
}