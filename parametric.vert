#pragma import_defines(Z_FUNCTION)

#ifndef Z_FUNCTION
    #define Z_FUNCTION(x, y) (x-x*x)*(y-y*y)*5.0
#endif

varying vec4 color;
varying vec4 v;

#ifdef Z_FUNCTION
vec4 computePosition(float x, float y)
{
    vec4 p = gl_Vertex;
    p.xyz += gl_Normal * Z_FUNCTION(x, y);
    return p;
}

vec3 computeNormal(float x, float y)
{
    float delta = 0.001;

    float x_left = x-delta;
    float x_right = x+delta;
    float y_below = y-delta;
    float y_above = y+delta;

    float z_left =  Z_FUNCTION(x_left, y);
    float z_right = Z_FUNCTION(x_right, y);
    float z_below = Z_FUNCTION(x, y_below);
    float z_above = Z_FUNCTION(x, y_above);

    vec3 x_delta = normalize(vec3(delta*2.0, 0.0, z_right-z_left));
    vec3 y_delta = normalize(vec3(0.0, delta*2.0, z_above-z_below));

    vec3 norm = cross(x_delta, y_delta);
    return normalize(norm);
}
#endif

void main(void)
{

#ifdef Z_FUNCTION
    v = computePosition( gl_Vertex.x, gl_Vertex.y);

    vec3 n = gl_NormalMatrix * computeNormal( gl_Vertex.x, gl_Vertex.y);

    vec4 lpos = gl_LightSource[0].position;

    vec3 lightDir;
    if (lpos.w == 0.0)
    {
        lightDir = lpos.xyz;
    }
    else
    {
        vec3 v_eye = (gl_ModelViewMatrix * v).xyz;
        lightDir = normalize(lpos.xyz - v_eye);
    }

    vec4 ambient_color = vec4(0.1, 0.1, 0.1, 0.0);
    vec4 diffuse_color = vec4(0.9, 0.9, 0.9, 0.0);

    float intensity = dot(lightDir, n);
    if (intensity<0.0) color = ambient_color;
    else color = ambient_color + diffuse_color * intensity;

    color.a = 1.0;

#else
    v = gl_Vertex;
    color = vec4(1.0,1.0,1.0,1.0);
#endif

    gl_Position = gl_ModelViewProjectionMatrix * v;
};
