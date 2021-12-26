#define FLECS_CITY_IMPL

#include <city.h>

/* Constants */
#define PAVEMENT_HEIGHT (0.3f)
#define PARK_HEIGHT (0.8f)
#define PLAZA_HEIGHT (1.2f)
#define PLAZA_POLE_HEIGHT (6.0f)
#define STREETS_HEIGHT (10.0f)
#define TRAFFIC_FREQUENCY (10.0f)

typedef struct {
    float width;
    float height;
    float left;
    float right;
    float top;
    float bottom;
} CityBound;

/* Internal component */
typedef struct {
    float elapsed;
    EcsVelocity3 initial_velocity;
    EcsRotation3 initial_rotation;
    CityBound bound; 
    float frequency;
} CityTrafficEmitter;

ECS_COMPONENT_DECLARE(CityTrafficEmitter);
ECS_COMPONENT_DECLARE(CityBound);

/* Resources loaded from module file */
static ecs_entity_t CityStreet;
static ecs_entity_t CityPavement;
static ecs_entity_t CityAc;
static ecs_entity_t CityTree;
static ecs_entity_t CitySidewalkTree;
static ecs_entity_t CityBush;
static ecs_entity_t CityLantern;
static ecs_entity_t CityPark;
static ecs_entity_t CityPlaza;
static ecs_entity_t CityBuilding;
static ecs_entity_t CityModernBuilding;
static ecs_entity_t CityCar;

/* Internal types */
typedef struct {
    ecs_entity_t city;
    float x, y;
    float building_size;
} CityBlock;

/* Utilities */
static
float randf(float scale) {
    return scale * ((float)rand() / (float)RAND_MAX);
}

static
float srandf(float scale) {
    return scale * ((float)rand() / (float)RAND_MAX) - scale / 2.0;
}

static
bool rolldice(float chance) {
    return randf(1.0) < chance;
}

static
ecs_entity_t plant_prop(ecs_world_t *world, CityBlock *block, float x, float y, ecs_entity_t prop) {
    ecs_entity_t e = ecs_new_w_pair(world, EcsChildOf, block->city);
    ecs_add_pair(world, e, EcsIsA, prop);
    ecs_set(world, e, EcsPosition3, {
        .x = block->x + x,
        .y = -0.1, 
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
void plant_park(ecs_world_t *world, CityBlock *block, float x, float y) {
    ecs_entity_t prop = plant_prop(world, block, x, y, CityPark);
    ecs_set(world, prop, EcsBox, {
        block->building_size, PARK_HEIGHT, block->building_size
    });
}

static
void plant_plaza(ecs_world_t *world, CityBlock *block, float x, float y) {
    ecs_entity_t prop = plant_prop(world, block, x, y, CityPlaza);
    ecs_set(world, prop, EcsBox, {
        block->building_size, PLAZA_HEIGHT, block->building_size
    });

    prop = plant_prop(world, block, x, y, CityPlaza);
    ecs_set(world, prop, EcsBox, {
        block->building_size * 0.75, PLAZA_HEIGHT * 2, block->building_size * 0.75
    });

    prop = plant_prop(world, block, x, y, CityPlaza);
    ecs_set(world, prop, EcsBox, {
        0.75, PLAZA_POLE_HEIGHT, 0.75
    });
    ecs_set(world, prop, EcsPosition3, {
        .x = block->x + x,
        .y = -0.1 - PLAZA_POLE_HEIGHT / 2.0, 
        .z = block->y + y
    });
}

static
ecs_entity_t plant_building_top(
    ecs_world_t *world, 
    ecs_entity_t city,
    float x, 
    float y, 
    float building_height,
    float top_height,
    float top_width,
    EcsRgb *color,
    bool modern) 
{
    ecs_entity_t top = ecs_new_w_pair(world, EcsChildOf, city);
    ecs_add_pair(world, top, EcsIsA, CityBuilding);
    ecs_set(world, top, EcsPosition3, {
        .x = x,
        .y = -building_height - (top_height / 2.0),
        .z = y
    });
    ecs_set(world, top, EcsBox, { top_width, top_height, top_width });
    ecs_set_ptr(world, top, EcsRgb, color);
    if (modern) ecs_add_pair(world, top, EcsIsA, CityModernBuilding);
    return top;
}

/* Generate city block */
void plant_city_block(
    ecs_world_t *world, 
    ecs_entity_t e, 
    const City *city, 
    int32_t x_i,
    int32_t y_i,
    int32_t x, 
    int32_t y) 
{
    float block_size = city->block_size;
    float building_size = city->buildings.size;

    float pave_width = (block_size - building_size) / 2.0;
    float corner = block_size / 2.0 - pave_width / 2.0;

    CityBlock block = {.city = e, .x = x, .y = y, .building_size = building_size};

    /* Create pavement */
    ecs_entity_t pavement = ecs_new_w_pair(world, EcsChildOf, e);
    ecs_add_pair(world, pavement, EcsIsA, CityPavement);
    ecs_set(world, pavement, EcsPosition3, {
        .x = x, 
        .y = -(PAVEMENT_HEIGHT / 2.0), 
        .z = y
    });
    ecs_set(world, pavement, EcsBox, {
        block_size, PAVEMENT_HEIGHT, block_size
    });

    if (!rolldice(city->parks.chance)) {
        /* Generate building */
        bool modern = false, skyscraper = false;

        /* Compute building height, tall buildings are less likely */
        float building_height = randf(1.0);
        building_height *= building_height;

        building_height = city->buildings.min_height + building_height * 
            city->buildings.max_height;

        if (rolldice(city->buildings.skyscraper_chance)) {
            building_height = building_height * (1.2 + randf(1.0));
            skyscraper = true;
        }

        ecs_entity_t building = ecs_new_w_pair(world, EcsChildOf, e);     
        ecs_add_pair(world, building, EcsIsA, CityBuilding);
        ecs_set(world, building, EcsPosition3, {
            .x = x,
            .y = -(building_height / 2.0),
            .z = y
        });
        ecs_set(world, building, EcsBox, {
            building_size, 
            building_height, 
            building_size
        });
        float bc = 0.1f + randf(0.7f);

        EcsRgb color = {bc, bc, bc};
        float color_chance = randf(1.0);

        float modern_chance = city->buildings.modern_chance;

        if (color_chance < modern_chance) {
            color = (EcsRgb){bc * 0.8, bc * 1.05, bc * 1.1};
            ecs_add_pair(world, building, EcsIsA, CityModernBuilding);
            modern = true;
        } else {
            color_chance = (color_chance - modern_chance) / (1.0 - modern_chance);
            if (color_chance < 0.5) {
                /* red brick */
                if (color_chance < 0.01 && building_height < 20) {
                    color = (EcsRgb){bc * 1.0, bc * 0.32, bc * 0.27};
                } else {
                    color = (EcsRgb){bc * 1.0, bc * 0.7, bc * 0.6};
                }
            }
        }

        ecs_set_ptr(world, building, EcsRgb, &color);

        if (building_height > 10 && randf(1.0) > 0.7) {
            plant_building_top(world, e, x, y, building_height, 
                building_height * 0.1, building_size * 0.8, &color, modern);

            if (randf(1.0) > 0.7) {
                plant_building_top(world, e, x, y, building_height, 
                    building_height * 0.2, building_size * 0.6, &color, modern);

                if (randf(1.0) > 0.7) {
                    plant_building_top(world, e, x, y, building_height, 
                        building_height * 0.3, building_size * 0.2, &color, modern);

                    if (randf(1.0) > 0.7 && skyscraper) {
                        plant_building_top(world, e, x, y, building_height, 
                            building_height * 0.6, building_size * 0.05, &color, modern);
                    }
                }
            }
        }
    } else {
        if (rolldice(city->parks.plaza_chance)) {
            plant_plaza(world, &block, 0, 0);
        } else {
            /* Generate park */
            plant_park(world, &block, 0, 0);
            
            if (rolldice(city->parks.tree_chance)) {
                if (rolldice(city->parks.tree_chance)) {
                    for (int t = 0; t < city->parks.tree_count; t ++) {
                        float tx = srandf(block_size - pave_width * 4);
                        float ty = srandf(block_size - pave_width * 4);
                        plant_tree(world, &block, tx, ty);
                    }
                }
            }
        }
    }

    /* Trees on the side of the road */
    if (rolldice(city->props.tree_chance)) {
        plant_sidewalk_tree(world, &block, -corner, 0);
    }
    if (rolldice(city->props.tree_chance)) {
        plant_sidewalk_tree(world, &block, corner, 0);
    }
    if (rolldice(city->props.tree_chance)) {
        plant_sidewalk_tree(world, &block, 0, -corner);
    }
    if (rolldice(city->props.tree_chance)) {
        plant_sidewalk_tree(world, &block, 0, corner);
    }

    /* Lanterns on street corners */
    if (city->lanterns) {
        if (!(x_i % 2) && !(y_i % 2)) {
            plant_lantern(world, &block, corner, corner);
            plant_lantern(world, &block, corner, -corner);
            plant_lantern(world, &block, -corner, -corner);
            plant_lantern(world, &block, -corner, corner);
        } else if (!((x_i + 1) % 2) && !((y_i + 1) % 2)) {
            plant_lantern(world, &block, corner, corner);
            plant_lantern(world, &block, corner, -corner);
            plant_lantern(world, &block, -corner, -corner);
            plant_lantern(world, &block, -corner, corner);
        }
    }
}

/* Generate city */
void SetCity(ecs_iter_t *it) {
    City *cities = ecs_term(it, City, 1);

    ecs_world_t *world = it->world;

    for (int i = 0; i < it->count; i ++) {
        ecs_entity_t e = it->entities[i];
        City *city = &cities[i];
        int blocks_x = city->blocks_x;
        int blocks_y = city->blocks_y;
        int min_height = city->buildings.min_height;
        int max_height = city->buildings.max_height;
        int block_size = city->block_size;
        int building_size = city->buildings.size;
        int road_width = city->road_width;
        float traffic_speed = city->traffic.speed;

        if (!blocks_x) {
           city->blocks_x = blocks_x = 10;
        }
        if (!blocks_y) {
            city->blocks_y = blocks_y = 10;
        }
        if (!min_height) {
            city->buildings.min_height = min_height = 5.0;
        }
        if (!max_height) {
            city->buildings.max_height = max_height = 20.0;
        }
        if (!block_size) {
            city->block_size = block_size = 20;
        }
        if (!building_size) {
            city->buildings.size = block_size - 5;
        }
        if (!road_width) {
            city->road_width = road_width = 10;
        }
        if (!traffic_speed) {
            traffic_speed = 15.0;
        }

        float width = (float)block_size + (float)road_width;

        float left = -((float)width * blocks_x) / 2.0f;
        float top = -((float)width * blocks_y) / 2.0f;
        left -= width / 2.0;
        top -= width / 2.0;

        CityBound bound = {
            .width = blocks_x * width, 
            .height = blocks_y * width,
            .left =    left  - width / 2.0,
            .right =   -left - width - width / 2.0,
            .top =     top  - width / 2.0,
            .bottom = -top - width - width / 2.0
        };

        ecs_entity_t streets = ecs_new_w_pair(world, EcsChildOf, e);
        ecs_set_name(world, streets, "Streets");
        ecs_add_pair(world, streets, EcsIsA, CityStreet);
        ecs_set(world, streets, EcsPosition3, {-width, STREETS_HEIGHT / 2.0, -width});
        ecs_set(world, streets, EcsBox, {
            blocks_x * width, STREETS_HEIGHT, blocks_y * width});

        for (int x = 0; x < blocks_x; x ++) {
            for (int y = 0; y < blocks_y; y ++) {
                plant_city_block(world, e, city, x, y,
                    left + width * x, 
                    top + width * y);
            }
        }

        if (city->traffic.frequency) {
            for (int x = 0; x < blocks_x; x ++) {
                float ex = left + width * x;

                ecs_entity_t emit_1 = ecs_new_id(world);
                ecs_entity_t emit_2 = ecs_new_id(world);
                ecs_set(world, emit_1, EcsPosition3, {
                    ex + width / 2.0 - road_width / 4.0, 0.0, top - width / 2.0
                });
                ecs_set(world, emit_1, CityTrafficEmitter, {
                    .initial_velocity = {0, 0.0, traffic_speed},
                    .initial_rotation = {0},
                    .bound = bound,
                    .frequency = city->traffic.frequency,
                    .elapsed = randf(city->traffic.frequency)
                });

                ecs_set(world, emit_2, EcsPosition3, {
                    ex - width / 2.0 + road_width / 4.0, 0.0, -top - width - width / 2.0
                });
                ecs_set(world, emit_2, CityTrafficEmitter, {
                    .initial_velocity = {0, 0.0, -traffic_speed},
                    .initial_rotation = {0, GLM_PI, 0},
                    .bound = bound,
                    .frequency = city->traffic.frequency,
                    .elapsed = randf(city->traffic.frequency)
                });
            }
        }
    }
}

static
void GenerateTraffic(ecs_iter_t *it) {
    EcsPosition3 *p = ecs_term(it, EcsPosition3, 1);
    CityTrafficEmitter *emit = ecs_term(it, CityTrafficEmitter, 2);

    ecs_world_t *world = it->world;

    for (int i = 0; i < it->count; i ++) {
        emit[i].elapsed += it->delta_time;

        if (emit[i].elapsed > emit[i].frequency) {
            ecs_entity_t e = ecs_new_id(world);
            ecs_set_ptr(world, e, EcsVelocity3, &emit[i].initial_velocity);
            ecs_set_ptr(world, e, EcsRotation3, &emit[i].initial_rotation);
            ecs_set_ptr(world, e, CityBound, &emit[i].bound);
            ecs_set_ptr(world, e, EcsPosition3, &p[i]);
            ecs_add_pair(world, e, EcsIsA, CityCar);
            emit[i].elapsed = 0;
        }
    }
}

static
void ExpireTraffic(ecs_iter_t *it) {
    EcsPosition3 *p = ecs_term(it, EcsPosition3, 1);
    CityBound *b = ecs_term(it, CityBound, 2);

    ecs_world_t *world = it->world;

    for (int i = 0; i < it->count; i ++) {
        ecs_entity_t e = it->entities[i];

        if(p[i].x < b[i].left) {
            ecs_delete(world, e);
        } else if(p[i].x > b[i].right) {
            ecs_delete(world, e);
        } else if(p[i].z < b[i].top) {
            ecs_delete(world, e);
        } else if(p[i].z > b[i].bottom) {
            ecs_delete(world, e);
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

    ecs_set_name_prefix(world, "City");

    ecs_set_name_prefix(world, NULL);

    ECS_COMPONENT_DEFINE(world, CityTrafficEmitter);
    ECS_COMPONENT_DEFINE(world, CityBound);
    ECS_META_COMPONENT(world, CityBuildings);
    ECS_META_COMPONENT(world, CityParks);
    ECS_META_COMPONENT(world, CityProps);
    ECS_META_COMPONENT(world, CityTraffic);
    ECS_META_COMPONENT(world, City);

    ecs_set_component_actions(world, City, {
        .on_set = SetCity
    });

    ECS_SYSTEM(world, GenerateTraffic, EcsOnUpdate, 
        [in]    flecs.components.transform.Position3,
        [inout] CityTrafficEmitter);

    ECS_SYSTEM(world, ExpireTraffic, EcsOnUpdate, 
        [in]    flecs.components.transform.Position3,
        [in]    CityBound);

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
        CityPlaza = ecs_lookup_fullpath(world, "Plaza");
        CityBuilding = ecs_lookup_fullpath(world, "Building");
        CityModernBuilding = ecs_lookup_fullpath(world, "ModernBuilding");
        CityCar = ecs_lookup_fullpath(world, "Car");
    } else {
        ecs_err("failed to load city assets");
    }
}

