
struct HitPayload
{
    bool hit;
    vec3 pos;
    vec3 normal;
    vec3 color;
    vec3 weight;
    vec3 nextDir;
    uint seed;
};

struct ShadowPayload
{
    bool hit;
};
