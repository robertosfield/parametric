#pragma import_defines(Z_FUNCTION, VISIBLE_FUNCTION)

#ifndef Z_FUNCTION
    #define Z_FUNCTION(x, y) (x-x*x)*(y-y*y)*5.0
#endif

#ifndef VISIBLE_FUNCTION
    #define VISIBLE_FUNCTION(x, y) (Z_FUNCTION(x,y)>cutOff)
#endif

uniform float cutOff;

varying vec4 color;
varying vec4 v;

void main(void)
{
#ifdef VISIBLE_FUNCTION
    if (!VISIBLE_FUNCTION(v.x, v.y)) discard;
#endif

    gl_FragColor = color;
};
