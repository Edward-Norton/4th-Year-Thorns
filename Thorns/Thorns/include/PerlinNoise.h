#ifndef PERLIN_NOISE_HPP
#define PERLIN_NOISE_HPP

#include <vector>
#include <random>
#include <cmath>

class PerlinNoise
{
public:
    

    
    PerlinNoise();

    
    explicit PerlinNoise(unsigned int seed);

    ~PerlinNoise() = default;

    

    
    
    double noise(double x, double y, double z = 0.0) const;

    

    
    
    
    
    
    double octaveNoise(double x, double y, double z,
        int octaves,
        double persistence = 0.5) const;

    

    
    double noise2D(double x, double y) const
    {
        return noise(x, y, 0.0);
    }

    
    double octaveNoise2D(double x, double y, int octaves, double persistence = 0.5) const
    {
        return octaveNoise(x, y, 0.0, octaves, persistence);
    }

private:
    

    
    double fade(double t) const;

    
    double lerp(double t, double a, double b) const;

    
    double grad(int hash, double x, double y, double z) const;

    

    
    
    std::vector<int> m_permutation;
};

#endif

