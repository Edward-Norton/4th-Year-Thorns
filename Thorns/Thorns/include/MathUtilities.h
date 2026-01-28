#ifndef MATH_UTILS_HPP
#define MATH_UTILS_HPP

#include <SFML/Graphics.hpp>

// Personal note: Need to use "inline" or you get a LNK2005 error due to multiple declaration rule.
// Inline makes it so the compiler is "hinted" to generate the code at the call site, in other words
// to emmbed the code every times its called, its used to bypass "One Definition Rule" at linking time when objects are made for tranlation units

namespace MathUtils
{
    // ===== MATH CONSTS =====
    inline constexpr float PI = 3.14159265358979323846;
    inline constexpr float TWO_PI = 2.0f * PI;
    inline constexpr float HALF_PI = 0.5f * PI;


    // ===== VECTOR OPERATIONS =====

    // Calculate the magnitude (length) of a 2D vector
    inline float magnitude(const sf::Vector2f& vector)
    {
        return std::sqrt(vector.x * vector.x + vector.y * vector.y);
    }

    // Normalize a vector to unit length (magnitude = 1)
    // Returns zero vector if input vector has zero magnitude
    inline sf::Vector2f normalize(const sf::Vector2f& vector)
    {
        float mag = magnitude(vector);
        return sf::Vector2f(vector.x / mag, vector.y / mag);
    }


    // ===== ANGLE OPERATIONS =====

    // Convert vector to angle in degrees (0° = right, 90° = down in SFML)
    inline float vectorToAngleDegrees(const sf::Vector2f& vector)
    {
        return std::atan2(vector.y, vector.x) * (180.0f / static_cast<float>(PI));
    }

    // Convert vector to angle in radians
    inline float vectorToAngleRadians(const sf::Vector2f& vector)
    {
        return std::atan2(vector.y, vector.x);
    }

    // Convert angle in degrees to unit direction vector
    inline sf::Vector2f angleDegreesToVector(float angleDegrees)
    {
        float angleRad = angleDegrees * (static_cast<float>(PI) / 180.0f);
        return sf::Vector2f(std::cos(angleRad), std::sin(angleRad));
    }

    // Convert angle in radians to unit direction vector  
    inline sf::Vector2f angleRadiansToVector(float angleRadians)
    {
        return sf::Vector2f(std::cos(angleRadians), std::sin(angleRadians));
    }


    // ===== ANGLE CONVERSION =====

    // Convert degrees to radians
    template<typename T>
    inline constexpr T degreesToRadians(T degrees)
    {
        return degrees * (static_cast<T>(M_PI) / T(180));
    }

    // Convert radians to degrees  
    template<typename T>
    inline constexpr T radiansToDegrees(T radians)
    {
        return radians * (T(180) / static_cast<T>(M_PI));
    }
}

#endif