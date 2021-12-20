#define FLECS_CITY_IMPL

#include <city.h>

#define PAVEMENT_HEIGHT (0.2f)
#define STREETS_HEIGHT (0.05f)

static ecs_entity_t CityStreet;
static ecs_entity_t CityPavement;
static ecs_entity_t CityAc;
static ecs_entity_t CityTree;
static ecs_entity_t CitySidewalkTree;
static ecs_entity_t CityBush;
static ecs_entity_t CityLantern;
static ecs_entity_t CityPark;

static
float randf(float scale) {
    return scale * ((float)rand() / (float)RAND_MAX);
}

static
float srandf(float scale) {
    return scale * ((float)rand() / (float)RAND_MAX) - scale / 2.0;
}

static
ecs_entity_t plant_prop(ecs_world_t *world, CityBlock *block, float x, float y, ecs_entity_t prop) {
    ecs_entity_t e = ecs_new_w_pair(world, EcsChildOf, block->city);
    ecs_add_pair(world, e, EcsIsA, prop);
    ecs_set(world, e, EcsPosition3, {
        .x = block->x + x,
        .y = 0, 
        .z = block->y + y
    });
    return e;
}

static
ecs_entity_t plant_tree(ecs_world_t *world, CityBlock *block, float x, float y) {
    return plant_prop(world, block, x, y, CityTree);
}

static
ecs_entity_t plant_bush(ecs_world_t *world, CityBlock *block, float x, float y) {
    return plant_prop(world, block, x, y, CityBush);
}

static
ecs_entity_t plant_sidewalk_tree(ecs_world_t *world, CityBlock *block, float x, float y) {
    return plant_prop(world, block, x, y, CitySidewalkTree);
}

static
ecs_entity_t plant_lantern(ecs_world_t *world, CityBlock *block, float x, float y) {
    return plant_prop(world, block, x, y, CityLantern);
}

static
ecs_entity_t plant_park(ecs_world_t *world, CityBlock *block, float x, float y) {
    return plant_prop(world, block, x, y, CityPark);
}

void SetCityBlock(ecs_iter_t *it) {
    CityBlock *blocks = ecs_term(it, CityBlock, 1);

    ecs_world_t *world = it->world;

    for (int i = 0; i < it->count; i ++) {
        ecs_entity_t e = it->entities[i];
        CityBlock *block = &blocks[i];

        float pave_width = block->size - block->building_size;
        float corner = (float)(block->size - (float)pave_width / 2.0) / 2.0;

        ecs_entity_t pavement = ecs_new_w_pair(world, EcsChildOf, block->city);
        ecs_add_pair(world, pavement, EcsIsA, CityPavement);
        ecs_set(world, pavement, EcsPosition3, {
            .x = block->x, 
            .y = -(PAVEMENT_HEIGHT / 2.0), 
            .z = block->y
        });
        ecs_set(world, pavement, EcsBox, {
            block->size, PAVEMENT_HEIGHT, block->size
        });

        if (randf(1.0) < (1.0 - block->park_chance)) {
            ecs_entity_t building = ecs_new_w_pair(world, EcsChildOf, block->city);        
            ecs_set(world, building, EcsPosition3, {
                .x = block->x,
                .y = -(block->building_height / 2.0),
                .z = block->y
            });
            ecs_set(world, e, EcsBox, {
                block->building_size, 
                block->building_height, 
                block->building_size
            });
            float bc = 0.3f + randf(0.5f);
            ecs_set(world, e, EcsRgb, {
                bc, bc, bc
            });

            if (randf(1.0) > 0.3) {
                ecs_entity_t ac = ecs_new_w_pair(world, EcsChildOf, block->city);
                ecs_add_pair(world, ac, EcsIsA, CityAc);
                ecs_set(world, ac, EcsPosition3, {
                    .x = block->x + srandf((float)block->building_size / 1.5),
                    .y = -(block->building_height / 2.0),
                    .z = block->y + srandf((float)block->building_size / 1.5)
                });
            }
        } else {
            ecs_entity_t park = plant_park(world, block, 0, 0);
            ecs_set(world, park, EcsBox, {
                block->size - pave_width, 0.7,
                block->size - pave_width
            });

            for (int t = 0; t < 5; t ++) {
                float tx = srandf(block->size - pave_width * 2.5);
                float ty = srandf(block->size - pave_width * 2.5);
                plant_tree(world, block, tx, ty);
            }
        }

        if (randf(1.0) < block->tree_chance) {
            plant_sidewalk_tree(world, block, -corner, 0);
        }
        if (randf(1.0) < block->tree_chance) {
            plant_sidewalk_tree(world, block, corner, 0);
        }
        if (randf(1.0) < block->tree_chance) {
            plant_sidewalk_tree(world, block, 0, -corner);
        }
        if (randf(1.0) < block->tree_chance) {
            plant_sidewalk_tree(world, block, 0, corner);
        }

        plant_lantern(world, block, corner, corner);
        plant_lantern(world, block, corner, -corner);
        plant_lantern(world, block, -corner, -corner);
        plant_lantern(world, block, -corner, corner);
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

        float width = (float)block_size + (float)road_width / 2.0;
        float left = -((float)width * blocks_x) / 2.0f;
        float top = -((float)width * blocks_y) / 2.0f;
        left -= width / 2.0;
        top -= width / 2.0;

        ecs_entity_t streets = ecs_new_w_pair(world, EcsChildOf, e);
        ecs_set_name(world, streets, "Streets");
        ecs_add_pair(world, streets, EcsIsA, CityStreet);
        ecs_set(world, streets, EcsPosition3, {-width, 0, -width});
        ecs_set(world, streets, EcsBox, {
            blocks_x * width, STREETS_HEIGHT, blocks_y * width});

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
                    .city = e,
                    .x = left + width * x,
                    .y = top + width * y,
                    .size = block_size,
                    .building_size = building_size,
                    .building_height = height,
                    .park_chance = city->park_chance,
                    .tree_chance = city->tree_chance,
                });
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

    /* Public components */
    ecs_set_name_prefix(world, "City");
    ECS_META_COMPONENT(world, CityBlock);
    ecs_set_name_prefix(world, NULL);
    ECS_META_COMPONENT(world, City);

    ECS_OBSERVER(world, SetCity, EcsOnSet, City);
    ECS_OBSERVER(world, SetCityBlock, EcsOnSet, CityBlock);

    /* Load module assets */
    if (ecs_plecs_from_file(world, "etc/assets/module.plecs") == 0) {
        CityStreet = ecs_lookup_fullpath(world, "Street");
        CityPavement = ecs_lookup_fullpath(world, "Pavement");
        CityAc = ecs_lookup_fullpath(world, "Ac");
        CityTree = ecs_lookup_fullpath(world, "Tree");
        CitySidewalkTree = ecs_lookup_fullpath(world, "SidewalkTree");
        CityBush = ecs_lookup_fullpath(world, "Bush");
        CityLantern = ecs_lookup_fullpath(world, "Lantern");
        CityPark = ecs_lookup_fullpath(world, "Park");
    } else {
        ecs_err("failed to load city assets");
    }
}
