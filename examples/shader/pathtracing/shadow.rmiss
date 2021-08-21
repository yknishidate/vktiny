#version 460
#extension GL_EXT_ray_tracing : enable
#include "globals.glsl"

layout(location = 1) rayPayloadInEXT ShadowPayload payLoad;

void main()
{
    payLoad.hit = false;
}
