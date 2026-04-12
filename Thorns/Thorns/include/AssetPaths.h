#ifndef ASSET_PATHS_H
#define ASSET_PATHS_H

#include <string>

namespace Assets
{
    
    namespace Textures
    {
        
        const std::string PLAYER_ATLAS = "ASSETS\\IMAGES\\player_atlas.png";
        const std::string ENEMY_SHIP = "ASSETS\\IMAGES\\EnemyShip.png";
        const std::string SAV_ENEMY = "ASSETS\\IMAGES\\savage.png";
        const std::string CHOMP_ENEMY = "ASSETS\\IMAGES\\chomper.png";

        
        const std::string TERRAIN_ATLAS = "ASSETS\\IMAGES\\terrain_atlas.png";

        
        const std::string HIDEOUT_SPRITE = "ASSETS\\IMAGES\\hideout_1.png";
        const std::string FARM_SPRITE_DEFAULT = "Farm_Default_Size.png";
        const std::string FARM_SPRITE = "ASSETS\\IMAGES\\Farm.png";

        
        const std::string FOREST_ATLAS = "ASSETS\\IMAGES\\forest_atlas.png";

        
        const std::string ITEMS_ATLAS = "ASSETS\\IMAGES\\items_atlas.png";
        
    }

    
    namespace Data
    {
        const std::string FOREST_ATLAS_DEFINITIONS = "ASSETS\\DATA\\forest-atlas-points.txt";
        const std::string FARM_DEFINITIONS = "ASSETS\\DATA\\farm-points.txt";
        const std::string ITEMS_ATLAS_DEFINITIONS = "ASSETS\\DATA\\items_atlas.txt";
    }

    
    namespace Maps
    {
        const std::string HIDEOUT_TEMPLATE = "ASSETS\\MAPS\\POI_Templates\\hideout.tmx";
        const std::string FARM_TEMPLATE = "ASSETS\\MAPS\\POI_Templates\\farm.tmx";
        const std::string WORLD_OBJECTS_TEMPLATE = "ASSETS\\MAPS\\OBJECT_TEMPLATES\\world_objects.tmx";
    }

    
    namespace Fonts
    {
        const std::string JERSEY_20 = "ASSETS\\FONTS\\Jersey20-Regular.ttf";
    }

    
    namespace Audio
    {

    }
}

#endif