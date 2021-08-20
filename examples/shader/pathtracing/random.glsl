
const highp float M_PI = 3.14159265358979323846;

uint pcg(inout uint state)
{
    uint prev = state * 747796405u + 2891336453u;
    uint word = ((prev >> ((prev >> 28u) + 4u)) ^ prev) * 277803737u;
    state     = prev;
    return (word >> 22u) ^ word;
}

uvec2 pcg2d(uvec2 v)
{
    v = v * 1664525u + 1013904223u;

    v.x += v.y * 1664525u;
    v.y += v.x * 1664525u;

    v = v ^ (v >> 16u);

    v.x += v.y * 1664525u;
    v.y += v.x * 1664525u;

    v = v ^ (v >> 16u);

    return v;
}

float rand(inout uint seed)
{
    uint val = pcg(seed);
    return (float(val) * (1.0 / float(0xffffffffu)));
}

vec3 cosineSampleHemisphere(float u1, float u2)
{
    vec3  dir;
    float r   = sqrt(u1);
    float phi = 2.0 * M_PI * u2;
    dir.x     = r * cos(phi);
    dir.y     = r * sin(phi);
    dir.z     = sqrt(max(0.0, 1.0 - dir.x * dir.x - dir.y * dir.y));

    return dir;
}

void createCoordinateSystem(in vec3 N, out vec3 Nt, out vec3 Nb)
{
    if(abs(N.x) > abs(N.y))
        Nt = vec3(N.z, 0, -N.x) / sqrt(N.x * N.x + N.z * N.z);
    else
        Nt = vec3(0, -N.z, N.y) / sqrt(N.y * N.y + N.z * N.z);
    Nb = cross(N, Nt);
}

vec3 sampleHemisphere(vec2 randVal, in vec3 x, in vec3 y, in vec3 z)
{
    float r1        = randVal.x;
    float r2        = randVal.y;
    vec3  direction = cosineSampleHemisphere(r1, r2);
    return direction.x * x + direction.y * y + direction.z * z;
}

vec3 sampleDirection(vec2 randVal, vec3 normal)
{
    vec3 tangent;
    vec3 bitangent;
    createCoordinateSystem(normal, tangent, bitangent);
    vec3 dir = cosineSampleHemisphere(randVal.x, randVal.y);
    return dir.x * tangent + dir.y * bitangent + dir.z * normal;
}
