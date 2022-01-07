#ifndef CITY_H
#define CITY_H

/* This generated file contains includes for project dependencies */
#include "city/bake_config.h"

#undef ECS_META_IMPL
#ifndef FLECS_CITY_IMPL
#define ECS_META_IMPL EXTERN // Ensure meta symbols are only defined once
#endif

#ifdef __cplusplus
extern "C" {
#endif

ECS_STRUCT(CityBuildings, {
    int16_t min_height;      
    int16_t max_height;
    int16_t min_width;
    float x_variation;
    float y_variation;
    float small_height;
    float skyscraper_chance;
    float modern_chance;
    float backyard_chance;
});

ECS_STRUCT(CityParks, {
    float chance;
    float tree_chance;
    float plaza_chance;
    int32_t tree_count;
});

ECS_STRUCT(CityProps, {
    float chance;
    float tree_chance;
    float bin_chance;
    float hydrant_chance;
    float bench_chance;
});

ECS_STRUCT(CityTraffic, {
    float frequency;
    float speed;
});

ECS_STRUCT(City, {
    int16_t blocks_x;
    int16_t blocks_y;
    int16_t block_width;
    int16_t block_height;
    int16_t road_width;
    int16_t pavement_width;

    bool lanterns;
    bool street_signs;

    CityBuildings buildings;
    CityParks parks;
    CityProps props;
    CityTraffic traffic;
});

void FlecsCityImport(
    ecs_world_t *world);

#ifdef __cplusplus
}
#endif

#endif
