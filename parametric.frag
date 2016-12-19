#pragma import_defines(Z_FUNCTION, VISIBLE_FUNCTION, NUM_DEPTH_TEXTURES)

#ifndef Z_FUNCTION
    #define Z_FUNCTION(x, y) (x-x*x)*(y-y*y)*5.0
#endif

#ifndef VISIBLE_FUNCTION
    #define VISIBLE_FUNCTION(x, y) (Z_FUNCTION(x,y)>cutOff)
#endif

#ifndef NUM_DEPTH_TEXTURES
    #define NUM_DEPTH_TEXTURES 0
#endif


uniform float cutOff;

varying vec4 color;
varying vec4 v;


#if NUM_DEPTH_TEXTURES>=1
uniform vec4 viewportDimensions;

uniform sampler2D frontDepthTexture0;
uniform sampler2D backDepthTexture0;
#endif

#if NUM_DEPTH_TEXTURES>=2
uniform sampler2D frontDepthTexture1;
uniform sampler2D backDepthTexture1;
#endif

void main(void)
{
#ifdef VISIBLE_FUNCTION
    if (!VISIBLE_FUNCTION(v.x, v.y)) discard;
#endif

#if NUM_DEPTH_TEXTURES>=1

    float depth = gl_FragCoord.z;

    vec2 texcoord = vec2((gl_FragCoord.x-viewportDimensions[0])/viewportDimensions[2], (gl_FragCoord.y-viewportDimensions[1])/viewportDimensions[3]);

    float backDepth0 = texture2D( backDepthTexture0, texcoord).s;
    if (depth>backDepth0)
    {
        // gl_FragColor = vec4(1.0,0.0,0.0,1.0);
        discard;
        // return;
    }

    float frontDepth0 = texture2D( frontDepthTexture0, texcoord).s;
    if (depth<frontDepth0)
    {
        // gl_FragColor = vec4(1.0,1.0,0.0,1.0);
        discard;
        //return;
    }

    #if NUM_DEPTH_TEXTURES>=2
    float backDepth1 = texture2D( backDepthTexture1, texcoord).s;
    float frontDepth1 = texture2D( frontDepthTexture1, texcoord).s;
    #endif



    gl_FragColor = color;
#else
    gl_FragColor = color;
#endif
};
