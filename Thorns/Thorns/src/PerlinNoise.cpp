#include "PerlinNoise.h"
#include <numeric>
#include <algorithm>
#include <iostream>

// ========================================================================================================
// CONSTRUCTION
// ========================================================================================================

PerlinNoise::PerlinNoise()
{
    // Initialize with Ken Perlin's reference permutation
    m_permutation.resize(256);

    // Fill with values 0-255
    std::iota(m_permutation.begin(), m_permutation.end(), 0);

    // Duplicate for easy wrapping (avoids modulo operations)
    m_permutation.insert(m_permutation.end(), m_permutation.begin(), m_permutation.end());
}

PerlinNoise::PerlinNoise(unsigned int seed)
{
    m_permutation.resize(256);

    // Fill with ordered sequence 0-255
    std::iota(m_permutation.begin(), m_permutation.end(), 0);

    // Shuffle using seed for reproducible randomness
    std::default_random_engine engine(seed);
    std::shuffle(m_permutation.begin(), m_permutation.end(), engine);

    // Duplicate for wrapping
    m_permutation.insert(m_permutation.end(), m_permutation.begin(), m_permutation.end());

    std::cout << "PerlinNoise initialized with seed: " << seed << "\n";
}

// ========================================================================================================
// NOISE GENERATION
// ========================================================================================================

double PerlinNoise::noise(double x, double y, double z) const
{
    // Find unit cube containing point
    int X = static_cast<int>(std::floor(x)) & 255;
    int Y = static_cast<int>(std::floor(y)) & 255;
    int Z = static_cast<int>(std::floor(z)) & 255;

    // Find relative position within cube [0,1]
    x -= std::floor(x);
    y -= std::floor(y);
    z -= std::floor(z);

    // Calculate fade curves for smooth interpolation
    double u = fade(x);
    double v = fade(y);
    double w = fade(z);

    // Hash coordinates of cube corners
    // This determines which gradient to use at each corner
    int A = m_permutation[X] + Y;
    int AA = m_permutation[A] + Z;
    int AB = m_permutation[A + 1] + Z;
    int B = m_permutation[X + 1] + Y;
    int BA = m_permutation[B] + Z;
    int BB = m_permutation[B + 1] + Z;

    // Interpolate along X axis
    double x1 = lerp(u, grad(m_permutation[AA], x, y, z),
        grad(m_permutation[BA], x - 1, y, z));
    double x2 = lerp(u, grad(m_permutation[AB], x, y - 1, z),
        grad(m_permutation[BB], x - 1, y - 1, z));
    double x3 = lerp(u, grad(m_permutation[AA + 1], x, y, z - 1),
        grad(m_permutation[BA + 1], x - 1, y, z - 1));
    double x4 = lerp(u, grad(m_permutation[AB + 1], x, y - 1, z - 1),
        grad(m_permutation[BB + 1], x - 1, y - 1, z - 1));

    // Interpolate along Y axis
    double y1 = lerp(v, x1, x2);
    double y2 = lerp(v, x3, x4);

    // Interpolate along Z axis
    double result = lerp(w, y1, y2);

    // Remap from [-1, 1] to [0, 1]
    return (result + 1.0) / 2.0;
}

double PerlinNoise::octaveNoise(double x, double y, double z,
    int octaves,
    double persistence) const
{
    double total = 0.0;
    double frequency = 1.0;
    double amplitude = 1.0;
    double maxValue = 0.0;  // Used for normalizing to [0,1]

    for (int i = 0; i < octaves; ++i)
    {
        // Sample at current frequency
        total += noise(x * frequency, y * frequency, z * frequency) * amplitude;

        // Track max possible value for normalization
        maxValue += amplitude;

        // Increase frequency (zoom in for detail)
        frequency *= 2.0;

        // Decrease amplitude (reduce influence)
        amplitude *= persistence;
    }

    // Normalize to [0, 1]
    return total / maxValue;
}

// ========================================================================================================
// HELPER FUNCTIONS
// ========================================================================================================

double PerlinNoise::fade(double t) const
{
    // Perlin's improved fade function: 6t^5 - 15t^4 + 10t^3
    // This gives smooth interpolation with zero derivative at t=0 and t=1
    // Removes visible grid artifacts
    return t * t * t * (t * (t * 6.0 - 15.0) + 10.0);
}

double PerlinNoise::lerp(double t, double a, double b) const
{
    // Linear interpolation: a + t * (b - a)
    return a + t * (b - a);
}

double PerlinNoise::grad(int hash, double x, double y, double z) const
{
    // Convert hash to gradient direction
    // Uses 12 gradient directions from cube center to edge midpoints
    // This is from Perlin's improved noise

    int h = hash & 15;  // Take lower 4 bits (0-15)

    // Convert to one of 12 gradient directions
    double u = (h < 8) ? x : y;
    double v = (h < 4) ? y : ((h == 12 || h == 14) ? x : z);

    // Randomly flip signs based on hash bits
    return ((h & 1) == 0 ? u : -u) + ((h & 2) == 0 ? v : -v);
}

// ========================================================================================================
// TECHNICAL NOTES
// ========================================================================================================
/*
 * WHY THESE SPECIFIC GRADIENTS?
 *
 * The 12 gradient vectors point from cube center to edge midpoints:
 * (1,1,0), (-1,1,0), (1,-1,0), (-1,-1,0)  // 4 directions in XY plane
 * (1,0,1), (-1,0,1), (1,0,-1), (-1,0,-1)  // 4 directions in XZ plane
 * (0,1,1), (0,-1,1), (0,1,-1), (0,-1,-1)  // 4 directions in YZ plane
 *
 * These are evenly distributed and eliminate directional bias artifacts
 * from the original Perlin noise implementation.
 *
 * PERFORMANCE NOTES:
 *
 * - Complexity: O(2^n) where n is dimensions (8 corners in 3D, 4 in 2D)
 * - The permutation table is accessed ~32 times per noise() call
 * - Octave noise multiplies this by number of octaves
 *
 * TYPICAL EXECUTION TIME (rough estimates):
 * - Single noise() call: ~0.5-1 microseconds
 * - 4-octave noise for 100x100 tiles: ~20-40ms
 * - This is acceptable for map generation but not real-time updates
 *
 * OPTIMIZATION OPPORTUNITIES:
 * - Use SIMD for vectorized fade/lerp operations
 * - Cache noise values in a texture/array
 * - Use GPU compute shader for large-scale generation
 */