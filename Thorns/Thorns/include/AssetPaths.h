#ifndef ASSET_PATHS_H
#define ASSET_PATHS_H

#include <string>

namespace Assets
{
    // ========== TEXTURES ==========
    namespace Textures
    {
        const std::string PLAYER_ATLAS = "ASSETS\\IMAGES\\player_atlas.png";
        const std::string ENEMY_SHIP = "ASSETS\\IMAGES\\EnemyShip.png";
        const std::string TERRAIN_ATLAS = "ASSETS\\IMAGES\\terrain_atlas.png";
        const std::string HIDEOUT_SPRITE = "ASSETS\\IMAGES\\hideout_1.png";
        const std::string FARM_SPRITE = "ASSETS\\IMAGES\\Farm.png";

        // Forest Objects
        const std::string FOREST_ATLAS = "ASSETS\\IMAGES\\forest_atlas.png";
        
    }

    // ========== DATA FILES ==========
    namespace Data
    {
        const std::string FOREST_ATLAS_DEFINITIONS = "ASSETS\\DATA\\forest-atlas-points.txt";
    }

    // ========== FONTS ==========
    namespace Fonts
    {
        const std::string JERSEY_20 = "ASSETS\\FONTS\\Jersey20-Regular.ttf";
    }

    // ========== AUDIO ==========
    namespace Audio
    {

    }
}

#endif