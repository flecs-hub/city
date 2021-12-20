#define FLECS_CITY_IMPL

#include <city.h>

#define PAVEMENT_HEIGHT (0.2f)
#define STREETS_HEIGHT (0.05f)

typedef struct {
    ecs_entity_t building;
    ecs_entity_t pavement;
} CityBlockData;

typedef struct {
    ecs_entity_t *blocks;
} CityData;

static ECS_COMPONENT_DECLARE(CityBlockData);
static ECS_COMPONENT_DECLARE(CityData);
static ecs_entity_t CityStreet;
static ecs_entity_t CityPavement;

void SetCityBuilding(ecs_iter_t *it) {
    CityBuilding *buildings = ecs_term(it, CityBuilding, 1);

    ecs_world_t *world = it->world;

    for (int i = 0; i < it->count; i ++) {
        ecs_entity_t e = it->entities[i];
        CityBuilding *building = &buildings[i];

        ecs_set(world, e, EcsPosition3, {
            0, -(building->height / 2.0), 0
        });

        ecs_set(world, e, EcsBox, {
            building->size, building->height, building->size
        });

        ecs_set(world, e, EcsRgb, {
            0.7, 0.7, 0.7
        });
    }
}

void SetCityBlock(ecs_iter_t *it) {
    CityBlock *blocks = ecs_term(it, CityBlock, 1);

    ecs_world_t *world = it->world;

    for (int i = 0; i < it->count; i ++) {
        ecs_entity_t e = it->entities[i];
        CityBlock *block = &blocks[i];

        CityBlockData *data = ecs_get_mut(world, e, CityBlockData, 0);
        ecs_entity_t pavement = ecs_new_w_pair(world, EcsChildOf, e);
        ecs_add_pair(world, pavement, EcsIsA, CityPavement);
        
        ecs_set(world, pavement, EcsPosition3, {
            0, -(PAVEMENT_HEIGHT / 2.0), 0
        });

        ecs_set(world, pavement, EcsBox, {
            block->size, PAVEMENT_HEIGHT, block->size
        });

        ecs_entity_t building = ecs_new_w_pair(world, EcsChildOf, e);
        ecs_set(world, building, CityBuilding, {
            .size = block->building_size,
            .height = block->building_height
        });
    }
}

void SetCity(ecs_iter_t *it) {
    City *cities = ecs_term(it, City, 1);

    ecs_world_t *world = it->world;

    for (int i = 0; i < it->count; i ++) {
        ecs_entity_t e = it->entities[i];
        City *city = &cities[i];
        int blocks_x = city->blocks_x;
        int blocks_y = city->blocks_y;
        int min_height = city->min_building_height;
        int max_height = city->max_building_height;
        int block_size = city->block_size;
        int building_size = city->building_size;
        int road_width = city->road_width;

        if (!blocks_x) {
            blocks_x = 10;
        }
        if (!blocks_y) {
            blocks_y = 10;
        }
        if (!min_height) {
            min_height = 5.0;
        }
        if (!max_height) {
            max_height = 20.0;
        }
        if (!block_size) {
            block_size = 20;
        }
        if (!building_size) {
            building_size = 15;
        }
        if (!road_width) {
            road_width = 10;
        }

        CityData *data = ecs_get_mut(world, e, CityData, 0);
        data->blocks = ecs_os_calloc_n(ecs_entity_t, blocks_x * blocks_y);

        float width = (float)block_size + (float)road_width / 2.0;
        float left = -((float)width * blocks_x) / 2.0f;
        float top = -((float)width * blocks_y) / 2.0f;
        left -= width / 2.0;
        top -= width / 2.0;

        ecs_entity_t streets = ecs_new_w_pair(world, EcsChildOf, e);
        ecs_set_name(world, streets, "Streets");
        ecs_add_pair(world, streets, EcsIsA, CityStreet);
        ecs_set(world, streets, EcsPosition3, {0, 0, 0});
        ecs_set(world, streets, EcsBox, {
            (2 + blocks_x) * width, STREETS_HEIGHT, (2 + blocks_y) * width});

        for (int x = 0; x < blocks_x; x ++) {
            for (int y = 0; y < blocks_y; y ++) {
                ecs_entity_t block = ecs_new_w_pair(world, EcsChildOf, e);

                float height = (float)rand() / (float)RAND_MAX;
                height = min_height + height * max_height;

                ecs_set(world, block, EcsPosition3, {
                    .x = left + width * x,
                    .z = top + width * y
                });

                ecs_set(world, block, CityBlock, {
                    .size = block_size,
                    .building_size = building_size,
                    .building_height = height
                });

                data->blocks[x + y * blocks_x] = block;
            }
        }
    }
}

void FlecsCityImport(
    ecs_world_t *world)
{
    ECS_MODULE(world, FlecsCity);

    ECS_IMPORT(world, FlecsComponentsTransform);
    ECS_IMPORT(world, FlecsComponentsGraphics);
    ECS_IMPORT(world, FlecsComponentsGeometry);

    /* Private components */
    ecs_set_name_prefix(world, "City");
    ECS_COMPONENT_DEFINE(world, CityData);
    ECS_COMPONENT_DEFINE(world, CityBlockData);

    /* Public components */
    ECS_META_COMPONENT(world, CityBuilding);
    ECS_META_COMPONENT(world, CityBlock);
    ecs_set_name_prefix(world, NULL);
    ECS_META_COMPONENT(world, City);

    ECS_OBSERVER(world, SetCity, EcsOnSet, City);
    ECS_OBSERVER(world, SetCityBlock, EcsOnSet, CityBlock);
    ECS_OBSERVER(world, SetCityBuilding, EcsOnSet, CityBuilding);

    /* Load module assets */
    if (ecs_plecs_from_file(world, "etc/assets/module.plecs") == 0) {
        CityStreet = ecs_lookup_fullpath(world, "Street");
        CityPavement = ecs_lookup_fullpath(world, "Pavement");
    } else {
        ecs_err("failed to load city assets");
    }
}
