#ifdef GL_ES
precision mediump float;
#endif

// Inputs
attribute vec4 a_start_positioin;
attribute vec4 a_end_position;
attribute float a_start_radio;
attribute float a_end_radio;
attribute float a_start_time;
attribute float a_end_time;

// Varyings
varying float v_start_radio;
varying float v_end_radio;
varying float v_start_time;
varying float v_end_time;


uniform float u_shadow_time;
uniform float u_unit_width;

/*
uniform vec4 u_color;
uniform float u_factor;
uniform float u_speed;
varying float Distance;*/

/*
vec4 qua_lerp(vec4 q1, vec4 q2, float t)
{
    if (t == 0.0) {
        return q1;
    } else if (t == 1.0) {
        return q2;
    }
    float t1 = 1.0 - t;
    vec4 dst;
    dst.x = t1 * q1.x + t * q2.x;
    dst.y = t1 * q1.y + t * q2.y;
    dst.z = t1 * q1.z + t * q2.z;
    dst.w = t1 * q1.w + t * q2.w;
    return dst;
}

vec3 rotate_vector( vec4 quat, vec3 vec )
{
    return vec + 2.0 * cross( cross( vec, quat.xyz ) + quat.w * vec, quat.xyz );
}*/

void main()
{
    /*
    vec4 ori = vec4(0,0,0,1);
    vec4 local_pos = a_origin_pos - a_center_pos;
    v_posture = qua_lerp(ori,a_posture,u_factor);
    gl_Position = CC_MVPMatrix * (a_target_pos * u_speed * u_factor + a_center_pos +  vec4(rotate_vector(v_posture, local_pos.xyz),local_pos.w));*/
    gl_Position = CC_MVPMatrix * (a_start_positioin + (a_end_position-a_start_positioin)*smoothstep(a_start_time, a_end_time, u_shadow_time));
    gl_PointSize = u_unit_width;
    v_start_radio = a_start_radio;
    v_end_radio = a_end_radio;
    v_start_time = a_start_time;
    v_end_time = a_end_time;
}