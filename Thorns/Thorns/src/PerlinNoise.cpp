#include "PerlinNoise.h"
#include <numeric>
#include <algorithm>
#include <iostream>

PerlinNoise::PerlinNoise()
{
    
    m_permutation.resize(256);

    std::iota(m_permutation.begin(), m_permutation.end(), 0);

    m_permutation.insert(m_permutation.end(), m_permutation.begin(), m_permutation.end());
}

PerlinNoise::PerlinNoise(unsigned int seed)
{
    m_permutation.resize(256);

    
    std::iota(m_permutation.begin(), m_permutation.end(), 0);

    
    std::default_random_engine engine(seed);
    std::shuffle(m_permutation.begin(), m_permutation.end(), engine);

    
    m_permutation.insert(m_permutation.end(), m_permutation.begin(), m_permutation.end());

    std::cout << "PerlinNoise initialized with seed: " << seed << "\n";
}

double PerlinNoise::noise(double x, double y, double z) const
{
    
    int X = static_cast<int>(std::floor(x)) & 255;
    int Y = static_cast<int>(std::floor(y)) & 255;
    int Z = static_cast<int>(std::floor(z)) & 255;

    
    x -= std::floor(x);
    y -= std::floor(y);
    z -= std::floor(z);

    
    double u = fade(x);
    double v = fade(y);
    double w = fade(z);

    
    
    int A = m_permutation[X] + Y;
    int AA = m_permutation[A] + Z;
    int AB = m_permutation[A + 1] + Z;
    int B = m_permutation[X + 1] + Y;
    int BA = m_permutation[B] + Z;
    int BB = m_permutation[B + 1] + Z;

    
    double x1 = lerp(u, grad(m_permutation[AA], x, y, z),
        grad(m_permutation[BA], x - 1, y, z));
    double x2 = lerp(u, grad(m_permutation[AB], x, y - 1, z),
        grad(m_permutation[BB], x - 1, y - 1, z));
    double x3 = lerp(u, grad(m_permutation[AA + 1], x, y, z - 1),
        grad(m_permutation[BA + 1], x - 1, y, z - 1));
    double x4 = lerp(u, grad(m_permutation[AB + 1], x, y - 1, z - 1),
        grad(m_permutation[BB + 1], x - 1, y - 1, z - 1));

    
    double y1 = lerp(v, x1, x2);
    double y2 = lerp(v, x3, x4);

    
    double result = lerp(w, y1, y2);

    
    return (result + 1.0) / 2.0;
}

double PerlinNoise::octaveNoise(double x, double y, double z,
    int octaves,
    double persistence) const
{
    double total = 0.0;
    double frequency = 1.0;
    double amplitude = 1.0;
    double maxValue = 0.0;  

    for (int i = 0; i < octaves; ++i)
    {
        
        total += noise(x * frequency, y * frequency, z * frequency) * amplitude;

        
        maxValue += amplitude;

        
        frequency *= 2.0;

        
        amplitude *= persistence;
    }

    
    return total / maxValue;
}

double PerlinNoise::fade(double t) const
{
    
    
    
    return t * t * t * (t * (t * 6.0 - 15.0) + 10.0);
}

double PerlinNoise::lerp(double t, double a, double b) const
{
    
    return a + t * (b - a);
}

double PerlinNoise::grad(int hash, double x, double y, double z) const
{
    
    
    

    int h = hash & 15;  

    
    double u = (h < 8) ? x : y;
    double v = (h < 4) ? y : ((h == 12 || h == 14) ? x : z);

    
    return ((h & 1) == 0 ? u : -u) + ((h & 2) == 0 ? v : -v);
}

