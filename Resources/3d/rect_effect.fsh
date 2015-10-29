
// Varyings
varying vec4 v_color;
varying vec2 v_radio_config;
varying float v_expand_start;


uniform float u_time;

//const float mingdu = 0.8;
//const vec3 centerColor = vec3(mingdu,mingdu,mingdu);

void main()
{
    vec2 coord = 2.0*(gl_PointCoord-vec2(0.5,0.5));
    float a = step(v_expand_start, u_time) - smoothstep(v_radio_config.x, v_radio_config.y, u_time);
    a = clamp(a, 0.0,1.0);
    vec4 baseCol = vec4(v_color.rgb, v_color.a *a);

    float len = max(abs(coord.x), abs(coord.y));
    vec4 resCol = mix(vec4(0.0,0.0,0.0,0.0),baseCol,  smoothstep(0.9,1.0,len));

    gl_FragColor = resCol;//vec4(mix(centerColor, v_color.rgb, u_is_outer), v_color.a * step(v_start_time, u_time)*(1.0 - step(v_end_time, u_time)));
}