

uniform sampler2D u_texture_background;
varying vec2 v_texcoord;

uniform vec4 u_para_ambient_color;

/* 与DDConfig.h内配置要同步 */
#define DD_LIGHT_NUM 15


//uniform float u_height;
uniform vec4 u_lights[DD_LIGHT_NUM];//{pos.x,pos.y,quantity,height}//这里的pox是光源->我的相对坐标，我们还需要就每个pixel的偏移做出修正。通过quantity取0来不显示阴影。
uniform vec4 u_lights_color[DD_LIGHT_NUM];//光源颜色

const float shininess = 32.0;
const float screenGamma = 2.2; // Assume the monitor is calibrated to the sRGB color space

vec3 computeLightingPerLight(vec3 diffuseColor, vec3 specularColor, vec3 normal, vec3 lightDir, float lightRadio)// norlmal和lightDir已经归一化，注意这个lightDir是从Point到光源的！
{
    float lambertian = max(dot(lightDir, normal), 0.0);
    float specular = 0.0;

    /* 无高光
    if (lambertian > 0.0) {
        vec3 viewDir = vec3(0.0,0.0,1.0);

        // this is blinn phong
        vec3 halfDir = normalize(lightDir + viewDir);
        float specAngle = max(dot(halfDir, normal), 0.0);
        specular = pow(specAngle, shininess);
    }*/
    vec3 colorLinear = (lambertian * diffuseColor + specular * specularColor) * lightRadio;

    return colorLinear;
}

vec3 computeLightingForAllLights(vec3 ambientColor, vec3 diffuseColor, vec3 normal)
{
    vec3 colorRes = vec3(0.0,0.0,0.0);
    for (int i = 0; i < DD_LIGHT_NUM; i++) {
        vec4 lightPara = u_lights[i];
        if (lightPara.z > 0.0) { //通过light的quanty来判断是否点亮
            vec2 newTexcoord = vec2(v_texcoord.x, 1.0 - v_texcoord.y);
            vec3 lightRelativePos = vec3(lightPara.xy + (newTexcoord - vec2(0.5,0.5))*(11.0), -lightPara.w);
            float len = (abs(lightRelativePos.x) + abs(lightRelativePos.y)+length(lightRelativePos) )*0.5;
            float lightRadio = max((1.0 - len/lightPara.z), 0.0);
            vec3 lightDir = normalize(-lightRelativePos);

            // 使用灯光作为散色光
            colorRes = colorRes + computeLightingPerLight(u_lights_color[i].rgb, u_lights_color[i].rgb, normal, lightDir, lightRadio);
        }
    }
    vec3 colorLinear = ambientColor + colorRes;
    vec3 colorGammaCorrected = pow(colorLinear, vec3(1.0/screenGamma));
    return colorGammaCorrected;
}

vec3 fetchNormalVector(vec2 texcoord)
{
    vec4 texcol = texture2D(u_texture_background, texcoord);
    return normalize(texcol.rgb - vec3(0.5,0.5,0.5));
}

void main()
{
    vec3 normalVector = fetchNormalVector(v_texcoord);
    vec3 lightRet = computeLightingForAllLights(vec3(0.0,0.0,0.0), u_para_ambient_color.rgb, normalVector);
    gl_FragColor = vec4(lightRet * 0.9, 1.0);
}