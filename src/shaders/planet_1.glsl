#version 330 core

out vec4 FragColor;

uniform vec2  uCircleCenter;  // screen px, GL origin (y up)
uniform float uCircleRadius;  // screen px (radius * camera.zoom)
uniform float uTime;          // same units as `time` in the C generator (+0.1 per frame)
uniform uint  u_seed;

// ------------------------------------------------------------
// Hash: same integer hash as the C version
// ------------------------------------------------------------

uint hash2(int x, int y)
{
    uint h = u_seed;
    h ^= uint(x) * 0x9E3779B1u;
    h  = (h << 13) | (h >> 19);
    h *= 0x85EBCA6Bu;
    h ^= uint(y) * 0xC2B2AE35u;
    h ^= h >> 16;  h *= 0x7FEB352Du;
    h ^= h >> 15;  h *= 0x846CA68Bu;
    h ^= h >> 16;
    return h;
}

// ------------------------------------------------------------
// Perlin noise: 8 fixed gradient directions, like the C version
// ------------------------------------------------------------

const vec2 GRAD[8] = vec2[8](
    vec2( 1.0,     0.0),
    vec2(-1.0,     0.0),
    vec2( 0.0,     1.0),
    vec2( 0.0,    -1.0),
    vec2( 0.7071,  0.7071),
    vec2(-0.7071,  0.7071),
    vec2( 0.7071, -0.7071),
    vec2(-0.7071, -0.7071)
);

vec2 gradientAt(int ix, int iy)
{
    return GRAD[hash2(ix, iy) & 7u];
}

float fade(float t)
{
    return ((6.0 * t - 15.0) * t + 10.0) * t * t * t;
}

float perlin2D(vec2 v)
{
    vec2 i = floor(v);
    vec2 f = v - i;
    int ix = int(i.x);
    int iy = int(i.y);

    float bl = dot(vec2(f.x,       f.y      ), gradientAt(ix,     iy    ));
    float tl = dot(vec2(f.x,       f.y - 1.0), gradientAt(ix,     iy + 1));
    float br = dot(vec2(f.x - 1.0, f.y      ), gradientAt(ix + 1, iy    ));
    float tr = dot(vec2(f.x - 1.0, f.y - 1.0), gradientAt(ix + 1, iy + 1));

    float dx = fade(f.x);
    float dy = fade(f.y);

    return mix(mix(bl, tl, dy), mix(br, tr, dy), dx);
}

// ------------------------------------------------------------
// fBm: rotate the input once, lacunarity 1.99, Hurst exponent H
// ------------------------------------------------------------

float fbm(vec2 p, float H, int octaves)
{
    float G = exp2(-H);

    float f    = 1.0;
    float a    = 1.0;
    float t    = 0.0;
    float aSum = 0.0;

    vec2 r = vec2(0.8 * p.x - 0.6 * p.y,
                  0.6 * p.x + 0.8 * p.y);

    for (int i = 0; i < octaves; i++) {
        t    += a * perlin2D(r * f);
        aSum += a;
        f    *= 1.99;
        a    *= G;
    }

    return t / aSum;
}

// ------------------------------------------------------------
// Three-layer sun field.
// d is -1..1 across the disc. The scale/offset constants are the
// C version's frequencies (0.0012, 0.0040, 0.0090) times 250.
// ------------------------------------------------------------

float sunField(vec2 d)
{
    // 1. Large scale structure
    vec2 q1 = d * 0.30 + 0.30;
    vec2 v1 = vec2(
        fbm(vec2(q1.x + uTime * 0.10,       q1.y),       0.75, 3),
        fbm(vec2(q1.x + 5.2 + uTime * 0.12, q1.y + 1.3), 0.75, 3)
    );

    // 2. Medium scale convection
    vec2 q2 = d * 1.00 + 1.00;
    vec2 v2 = vec2(
        fbm(vec2(q2.x + 2.0 * v1.y + uTime * 0.20,
                 q2.y + 2.0 * v1.x), 0.50, 4),
        fbm(vec2(q2.x + 2.0 * v1.x + uTime * 0.23,
                 q2.y + 2.0 * v1.y), 0.50, 4)
    );

    // 3. Fine granulation
    vec2 q3 = d * 2.25 + 2.25;
    float value = fbm(vec2(q3.x + 2.0 * v2.y + uTime * 0.35,
                           q3.y + 2.0 * v2.x), 0.40, 5);

    float w = value * 0.5 + 0.5;
    w = smoothstep(0.25, 0.75, w);
    w = pow(w, 1.15);
    return clamp(w, 0.0, 1.0);
}

void main()
{
    vec2 d = (gl_FragCoord.xy - uCircleCenter) / uCircleRadius;
    d.y = -d.y;   // C image has row 0 at the top

    float w = sunField(d);

    FragColor = vec4(1.0,
                     0.20  + 0.75 * w,
                     0.015 + 0.06 * w,
                     1.0);
}
