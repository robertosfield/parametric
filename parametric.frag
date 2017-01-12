#pragma import_defines(NUM_DEPTH_TEXTURES)

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

#if NUM_DEPTH_TEXTURES>=3
uniform sampler2D frontDepthTexture2;
uniform sampler2D backDepthTexture2;
#endif

#if NUM_DEPTH_TEXTURES>=4
uniform sampler2D frontDepthTexture3;
uniform sampler2D backDepthTexture3;
#endif

bool outside(vec2 texcoord, float depth, sampler2D backDepthTexture, sampler2D frontDepthTexture)
{
    float backDepth0 = texture2D( backDepthTexture, texcoord).s;
    if (depth>backDepth0) return true;

    float frontDepth0 = texture2D( frontDepthTexture, texcoord).s;
    return (depth<frontDepth0);
}

void main(void)
{
#if NUM_DEPTH_TEXTURES>=1

    float depth = gl_FragCoord.z;

    vec2 texcoord = vec2((gl_FragCoord.x-viewportDimensions[0])/viewportDimensions[2], (gl_FragCoord.y-viewportDimensions[1])/viewportDimensions[3]);

    if (outside(texcoord, depth, backDepthTexture0, frontDepthTexture0)) discard;

    #if NUM_DEPTH_TEXTURES>=2
        if (outside(texcoord, depth, backDepthTexture1, frontDepthTexture1)) discard;
    #endif

    #if NUM_DEPTH_TEXTURES>=3
        if (outside(texcoord, depth, backDepthTexture2, frontDepthTexture2)) discard;
    #endif
#endif
    gl_FragColor = color;
};
