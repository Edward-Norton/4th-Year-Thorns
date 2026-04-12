#ifndef MATH_UTILS_HPP
#define MATH_UTILS_HPP

#include <SFML/Graphics.hpp>

namespace MathUtils
{
    
    inline constexpr float PI = 3.14159265358979323846;
    inline constexpr float TWO_PI = 2.0f * PI;
    inline constexpr float HALF_PI = 0.5f * PI;

    

    
    inline float magnitude(const sf::Vector2f& vector)
    {
        return std::sqrt(vector.x * vector.x + vector.y * vector.y);
    }

    
    
    inline sf::Vector2f normalize(const sf::Vector2f& vector)
    {
        float mag = magnitude(vector);
        return sf::Vector2f(vector.x / mag, vector.y / mag);
    }

    

    
    inline float vectorToAngleDegrees(const sf::Vector2f& vector)
    {
        return std::atan2(vector.y, vector.x) * (180.0f / static_cast<float>(PI));
    }

    
    inline float vectorToAngleRadians(const sf::Vector2f& vector)
    {
        return std::atan2(vector.y, vector.x);
    }

    
    inline sf::Vector2f angleDegreesToVector(float angleDegrees)
    {
        float angleRad = angleDegrees * (static_cast<float>(PI) / 180.0f);
        return sf::Vector2f(std::cos(angleRad), std::sin(angleRad));
    }

    
    inline sf::Vector2f angleRadiansToVector(float angleRadians)
    {
        return sf::Vector2f(std::cos(angleRadians), std::sin(angleRadians));
    }

    

    
    template<typename T>
    inline constexpr T degreesToRadians(T degrees)
    {
        return degrees * (static_cast<T>(M_PI) / T(180));
    }

    
    template<typename T>
    inline constexpr T radiansToDegrees(T radians)
    {
        return radians * (T(180) / static_cast<T>(M_PI));
    }
}

#endif