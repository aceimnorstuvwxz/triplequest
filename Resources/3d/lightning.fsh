
// Varyings
varying vec4 v_color;
varying float v_start_time;
varying float v_end_time;


uniform float u_time;
uniform float u_is_outer;

const float mingdu = 0.8;
const vec3 centerColor = vec3(mingdu,mingdu,mingdu);

void main()
{
    gl_FragColor = vec4(mix(centerColor, v_color.rgb, u_is_outer), v_color.a * step(v_start_time, u_time)*(1.0 - step(v_end_time, u_time)));
}