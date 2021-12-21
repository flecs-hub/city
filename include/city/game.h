#ifndef FLECS_CITY_H
#define FLECS_CITY_H

#ifdef __cplusplus
extern "C" {
#endif

ECS_STRUCT(CityBlock, {
    float x;
    float y;
    ecs_entity_t city;
    int16_t size;
    int16_t building_size;
    int16_t building_height;
    float park_chance;
    float tree_chance;
});

ECS_STRUCT(City, {
    int16_t blocks_x;
    int16_t blocks_y;
    int16_t block_size;
    int16_t road_width;
    int16_t building_size;
    int16_t min_building_height;
    int16_t max_building_height;
    float park_chance;
    float tree_chance;
});

void FlecsCityImport(
    ecs_world_t *world);

#ifdef __cplusplus
}
#endif
