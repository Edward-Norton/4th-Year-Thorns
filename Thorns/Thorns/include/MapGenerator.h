#ifndef MAP_GENERATOR_HPP
#define MAP_GENERATOR_HPP

#include <memory>
#include "Map.h"
#include "VoronoiDiagram.h"
#include "ObjectPlacer.h"
#include "POITemplate.h"
#include "POITypeConfig.h"

class MapGenerator
{
public:
    enum class SiteDensity
    {
        Sparce,  
        Medium, 
        Dense,  
    };

    struct GenerationSettings
    {
        
        unsigned short mapWidth = 128 ;          
        unsigned short mapHeight = 128;         
        float tileSize = 64.f;       

        
        
        
        
        struct ManualSites { unsigned char count = 20; }; 
        struct AutoDensity { SiteDensity density = SiteDensity::Medium; }; 

        
        
        std::variant<ManualSites, AutoDensity> siteMode = ManualSites{};
 
        float minSiteDistance = 0.0f; 
        unsigned int seed = 0;       

        
        unsigned char numVillages = 1;
        unsigned char numFarms = 1;

        
        bool enableObjectPlacement = true;      
        double objectFrequency = 0.08;          
        int objectOctaves = 2;                  
        double objectThreshold = 0.65;          

        
        
        
        float deriveMinSiteDistance(int count) const
        {
            float worldW = static_cast<float>(mapWidth) * tileSize;
            float worldH = static_cast<float>(mapHeight) * tileSize;

            
            
            return std::sqrt((worldW * worldH) / static_cast<float>(std::max(1, count))) * 0.85f;
        }

    };

    MapGenerator();
    ~MapGenerator() = default;

    
    
    std::unique_ptr<Map> generate(const GenerationSettings& settings);

    
    void regenerate(Map* map, const GenerationSettings& settings);

    
    VoronoiDiagram* getVoronoiDiagram() { return m_voronoi.get(); }
    ObjectPlacer* getObjectPlacer() { return m_objectPlacer.get(); }

    
    std::vector<sf::Vector2f> getEnemySpawnPoints(int countPerPOI = 2, float spawnRadius = 300.f) const;
    
    std::vector<sf::Vector2f> getItemSpawnPoints(float spawnRadius = 200.f) const;

private:
    
    void phase1_Voronoi(Map* map, const GenerationSettings& settings);
    void phase2_PerlinObjects(Map* map, const GenerationSettings& settings);

    
    unsigned char calSiteOptimalCount(SiteDensity density, const GenerationSettings& settings) const;
    
    float getAreaPerSite(SiteDensity density) const;

    
    
    void setupHideoutPOI(Map* map);

    
    void spawnPOIsAtSites(Map* map, const GenerationSettings& settings);

    POIConfigRegistry m_poiConfig;
    POITemplateManager m_poiTemplates;

    std::unique_ptr<PointOfInterest> createPOI(
        PointOfInterest::Type type,
        const sf::Vector2f& position,
        int instanceNumber);

    
    PointOfInterest::Type getRandomPOIType(int& villagesLeft, int& farmsLeft,std::mt19937& rng);

    
    std::unique_ptr<VoronoiDiagram> m_voronoi;
    std::unique_ptr<ObjectPlacer> m_objectPlacer;

    
    sf::Vector2f m_hideoutPosition;

    
    bool m_perlinAssetsInit = false;

};

#endif