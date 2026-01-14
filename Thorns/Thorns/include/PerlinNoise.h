#ifndef PERLIN_NOISE_HPP
#define PERLIN_NOISE_HPP

#include <vector>
#include <random>
#include <cmath>

/// <summary>
/// Perlin Noise implementation
/// 
/// Used for procedural object placement and terrain variation
/// 
/// 
/// Key Concepts:
/// - Gradient Noise: Uses pseudo-random gradient vectors at grid points
/// - Smooth Interpolation: Fade function creates smooth transitions
/// - Octaves: Multiple layers of noise at different frequencies for detail
/// - Persistence: Controls amplitude decrease per octave (typically 0.5)
/// - Frequency: Controls spacing of noise features
/// 
/// Usage for Object Placement:
/// 1. Generate noise value for each tile position
/// 2. Use threshold to determine if object should be placed
/// 3. Use noise value to select object variant/type
/// </summary>
class PerlinNoise
{
public:
    // ========== Construction ==========

    // Initialize with default permutation vector
    PerlinNoise();

    // Initialize with custom seed for reproducible generation
    explicit PerlinNoise(unsigned int seed);

    ~PerlinNoise() = default;

    // ========== Noise Generation ==========

    /// Generate noise value at 2D coordinates
    /// Returns value in range [0.0, 1.0]
    double noise(double x, double y, double z = 0.0) const;

    // ========== Octave Noise ==========

    /// Generate multi-octave noise for more detail
    /// x, y, z - Position coordinates
    /// octaves - Number of noise layers to combine (1-8 typical)
    /// persistence - Amplitude multiplier per octave (0.5 typical)
    /// Noise value in range [0.0, 1.0]
    double octaveNoise(double x, double y, double z,
        int octaves,
        double persistence = 0.5) const;

    // ========== 2D Convenience Methods ==========

    /// Generate 2D noise (z = 0)
    double noise2D(double x, double y) const
    {
        return noise(x, y, 0.0);
    }

    /// Generate 2D octave noise
    double octaveNoise2D(double x, double y, int octaves, double persistence = 0.5) const
    {
        return octaveNoise(x, y, 0.0, octaves, persistence);
    }

private:
    // ========== Helper Functions ==========

    /// Fade function for smooth interpolation
    double fade(double t) const;

    /// Linear interpolation
    double lerp(double t, double a, double b) const;

    /// Gradient function - dot product with pseudo-random gradient
    double grad(int hash, double x, double y, double z) const;

    // ========== Data ==========

    /// Permutation vector
    /// Used to generate pseudo-random gradients at grid points
    std::vector<int> m_permutation;
};

#endif

// Personal Notes Perlin Noise:
/*
 * /// References:
        - Ken Perlin's Improved Noise: http://mrl.nyu.edu/~perlin/paper445.pdf
        - Implementation based on: https://solarianprogrammer.com/2012/07/18/perlin-noise-cpp-11/ 
 * PERLIN NOISE ALGORITHM OVERVIEW:
 *
 * 1. Grid System:
 *    - Space is divided into unit cubes
 *    - Each grid point has a pseudo-random gradient vector
 *
 * 2. For each sample point:
 *    a) Find containing unit cube
 *    b) Calculate relative position within cube [0,1]
 *    c) Get gradients at 8 corners (4 in 2D)
 *    d) Compute dot products of gradients with offset vectors
 *    e) Interpolate results using smooth fade function
 *
 * 3. Octaves (Fractal Brownian Motion):
 *    - Combine multiple noise layers at different scales
 *    - Each octave has 2x frequency and roughly 0.5x amplitude
 *    - Creates natural-looking variation at multiple scales
 */