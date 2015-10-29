#ifdef GL_ES
precision mediump float;
#endif

// Inputs
attribute vec4 a_positioin;
attribute vec2 a_texcoord;


varying vec2 v_texcoord;



void main()
{
    gl_Position = CC_MVPMatrix * a_positioin;
    v_texcoord = a_texcoord;
}