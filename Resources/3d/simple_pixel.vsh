#ifdef GL_ES
precision mediump float;
#endif

// Inputs
attribute vec4 a_positioin;
attribute vec3 a_color;

// Varyings
#ifdef GL_ES
varying mediump vec3 v_color;
#endif


void main()
{
    gl_Position = CC_MVPMatrix * a_positioin;
    v_color = a_color;
}