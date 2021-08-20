#version 460
#extension GL_EXT_ray_tracing : enable
#include "globals.glsl"

layout(location = 0) rayPayloadInEXT HitPayload payLoad;

void main()
{
    payLoad.hit = false;
    payLoad.color *= 5.0;
}
