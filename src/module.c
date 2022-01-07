#define FLECS_CITY_IMPL

#include <city.h>

/* Constants */
#define PAVEMENT_HEIGHT (0.3f)
#define PARK_HEIGHT (0.2f)
#define PLAZA_HEIGHT (1.2f)
#define PLAZA_POLE_HEIGHT (6.0f)
#define STREETS_HEIGHT (10.0f)
#define TRAFFIC_FREQUENCY (10.0f)
#define SMALL_BUILDING_THRESHOLD (20.0f)

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
static ecs_entity_t CityBin;
static ecs_entity_t CityHydrant;
static ecs_entity_t CityLantern;
static ecs_entity_t CityBench;
static ecs_entity_t CityPark;
static ecs_entity_t CityPlaza;
static ecs_entity_t CityBuilding;
static ecs_entity_t CityModernBuilding;
static ecs_entity_t CityCar;

/* Internal types */
typedef struct {
    const City *city;
    ecs_entity_t parent;
    float x, y;
    float build_area_width;
    float build_area_height;
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
ecs_entity_t plant_object(ecs_world_t *world, CityBlock *block, float x, float y, ecs_entity_t prop) {
    ecs_entity_t e = ecs_new_w_pair(world, EcsChildOf, block->parent);
    ecs_add_pair(world, e, EcsIsA, prop);
    ecs_set(world, e, EcsPosition3, {
        .x = block->x + x,
        .y = -PAVEMENT_HEIGHT,
        .z = block->y + y
    });
    return e;
}

static
ecs_entity_t plant_tree(ecs_world_t *world, CityBlock *block, float x, float y) {
    return plant_object(world, block, x, y, CityTree);
}

static
ecs_entity_t plant_prop(ecs_world_t *world, CityBlock *block, float x, float y) {
    const CityProps *props = &block->city->props;
    ecs_entity_t result = 0;

    float p_sum = 
        props->tree_chance + 
        props->bin_chance + 
        props->hydrant_chance +
        props->bench_chance;

    float p = props->tree_chance;
    float dice = randf(1.0) * p_sum;

    if (dice < p) {
        result += plant_object(world, block, x, y, CitySidewalkTree);
    }
    
    p += props->bin_chance;
    if (!result && dice < p) {
       result =  plant_object(world, block, x, y, CityBin);
    }

    p += props->hydrant_chance;
    if (!result && dice < p) {
        result = plant_object(world, block, x, y, CityHydrant);
    }

    p += props->bench_chance;
    if (!result && dice < p) {
        result = plant_object(world, block, x, y, CityBench);
    }

    if (x < 0) {
        ecs_set(world, result, EcsRotation3, {0, M_PI / 2.0, 0});
    } else if (x > 0) {
        ecs_set(world, result, EcsRotation3, {0, -M_PI / 2.0, 0});
    } else if (y > 0) {
        ecs_set(world, result, EcsRotation3, {0, M_PI, 0});
    }

    return result;
}

static
ecs_entity_t plant_lantern(ecs_world_t *world, CityBlock *block, float x, float y) {
    return plant_object(world, block, x, y, CityLantern);
}

static
void plant_park(ecs_world_t *world, CityBlock *block, float x, float y) {
    ecs_entity_t prop = plant_object(world, block, x, y, CityPark);
    ecs_set(world, prop, EcsBox, {
        block->build_area_width, PARK_HEIGHT, block->build_area_height
    });
}

static
void plant_backyard(ecs_world_t *world, CityBlock *block, float x, float y) {
    ecs_entity_t prop = plant_object(world, block, x, y, CityPark);
    ecs_set(world, prop, EcsBox, {
        block->build_area_width - 5.0, 
        PARK_HEIGHT, 
        block->build_area_height - 5.0
    });
}

static
void plant_plaza(ecs_world_t *world, CityBlock *block, float x, float y) {
    ecs_entity_t prop = plant_object(world, block, x, y, CityPlaza);
    ecs_set(world, prop, EcsBox, {
        block->build_area_width, PLAZA_HEIGHT, block->build_area_height
    });

    prop = plant_object(world, block, x, y, CityPlaza);
    ecs_set(world, prop, EcsBox, {
        block->build_area_width * 0.75, PLAZA_HEIGHT * 2, block->build_area_height * 0.75
    });

    prop = plant_object(world, block, x, y, CityPlaza);
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
    float top_x,
    float top_y,
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
    ecs_set(world, top, EcsBox, { top_x, top_height, top_y });
    ecs_set_ptr(world, top, EcsRgb, color);
    if (modern) ecs_add_pair(world, top, EcsIsA, CityModernBuilding);
    return top;
}

static
void plant_building(
    ecs_world_t *world,
    const City *city,
    ecs_entity_t parent,
    float x,
    float y,
    float building_height,
    float x_size,
    float y_size,
    bool modern,
    bool skyscraper)
{
    /* Generate building */
    ecs_entity_t building = ecs_new_w_pair(world, EcsChildOf, parent);     
    ecs_add_pair(world, building, EcsIsA, CityBuilding);
    ecs_set(world, building, EcsPosition3, {
        .x = x,
        .y = -(building_height / 2.0),
        .z = y
    });
    ecs_set(world, building, EcsBox, {
        x_size, 
        building_height, 
        y_size
    });
    float bc = 0.1f + randf(0.7f);

    EcsRgb color = {bc, bc, bc};

    if (modern) {
        color = (EcsRgb){bc * 0.8, bc * 1.05, bc * 1.1};
        ecs_add_pair(world, building, EcsIsA, CityModernBuilding);
        modern = true;
    } else {
        float color_chance = randf(1.0);
        if (color_chance < 0.2) {
            /* red brick */
            if (color_chance < 0.01 && building_height < 20) {
                color = (EcsRgb){bc * 1.0, bc * 0.32, bc * 0.27};
            } else {
                color = (EcsRgb){bc * 1.0, bc * 0.7, bc * 0.6};
            }
        }
    }

    ecs_set_ptr(world, building, EcsRgb, &color);

    if (building_height > city->buildings.small_height && randf(1.0) > 0.8) {
        float top_height = 10.0 * (randf(0.5) + 0.5);
        float top_dw = x_size * 0.3;
        top_height = glm_min(top_height, building_height * 0.05);

        plant_building_top(world, parent, x, y, building_height, 
            top_height, x_size - top_dw, y_size - top_dw, 
            &color, modern);

        if (randf(1.0) > 0.7) {
            top_dw = x_size * 0.6;
            plant_building_top(world, parent, x, y, building_height, 
                top_height * 2.0, x_size - top_dw, x_size - top_dw, 
                &color, modern);

            if (randf(1.0) > 0.5 && building_height > 35.0) {
                top_dw = x_size * 0.9;
                plant_building_top(world, parent, x, y, building_height, 
                    top_height * 3.0, x_size - top_dw, x_size - top_dw, 
                    &color, modern);

                if (randf(1.0) > 0.3 && skyscraper) {
                    top_dw = x_size * 0.96;
                    plant_building_top(world, parent, x, y, building_height, 
                        top_height * 6.0, x_size - top_dw, x_size - top_dw, 
                        &color, modern);
                }
            }
        }
    }
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
    float block_width = city->block_width;
    float block_height = city->block_height;
    float pave_width = city->pavement_width;
    float corner_w = block_width / 2.0 - pave_width / 2.0;
    float corner_h = block_height / 2.0 - pave_width / 2.0;
    float build_area_width = block_width - pave_width * 2;
    float build_area_height = block_height - pave_width * 2;
    float build_area_ratio = build_area_width / build_area_height;
    float building_min_width = city->buildings.min_width;

    CityBlock block = {
        .city = city,
        .parent = e,
        .x = x, 
        .y = y, 
        .build_area_width = build_area_width,
        .build_area_height = build_area_height
    };

    /* Create pavement */
    ecs_entity_t pavement = ecs_new_w_pair(world, EcsChildOf, e);
    ecs_add_pair(world, pavement, EcsIsA, CityPavement);
    ecs_set(world, pavement, EcsPosition3, {
        .x = x, 
        .y = -(PAVEMENT_HEIGHT / 2.0), 
        .z = y
    });
    ecs_set(world, pavement, EcsBox, {
        block_width, PAVEMENT_HEIGHT, block_height
    });

    if (!rolldice(city->parks.chance)) {
        float min_height = city->buildings.min_height;
        float max_height = city->buildings.max_height;

        /* Compute distsance to center, which hosts the most large buildings */
        float max_dx = city->blocks_x / 2.0;
        float max_dy = city->blocks_y / 2.0;
        float dx = (x_i - max_dx) / max_dx;
        float dy = (y_i - max_dy) / max_dy;
        float d = 1.0 - sqrt(dx * dx + dy * dy);

        /* Exponential falloff */
        d *= d;

        /* Add some variability so the falloff isn't too predictable */
        d = randf(0.2) + d * 0.8;

        /* Don't apply correction for the inner center */
        if (d >= 0.7) {
            d = 1.0;
        }

        float height_v = (max_height - min_height); /* max variation */
        float height = min_height + height_v * randf(1.0) * d;
        float x_size = build_area_width;
        float y_size = build_area_height;

        if (height > city->buildings.small_height) {
            bool modern = rolldice(city->buildings.modern_chance);
            
            bool skyscraper = false;
            if (height > max_height * 0.7) {
                skyscraper = rolldice(city->buildings.skyscraper_chance);
                if (skyscraper) {
                    height = height * (1.2 + randf(1.0));
                }
            }

            plant_building(world, city, e, x, y, height, x_size, 
                y_size, modern, skyscraper);
        } else {
            float x_size, y_size = building_min_width + 
                (build_area_height - building_min_width) * (0.5 * randf(0.5));

            int bx, num_x = 2.0;
            x_size = build_area_width / (float)num_x;

            y_size = glm_min(x_size, y_size);

            int by, num_y = build_area_height / y_size;
            y_size = build_area_height / (float)num_y;

            float x_start = x - build_area_width / 2.0 + x_size / 2.0;
            float y_start = y - build_area_height / 2.0 + y_size / 2.0;

            bool backyard = false;
            for (bx = 0; bx < num_x; bx ++) {
                float y_delta = 0;

                for (by = 0; by < num_y; by ++) {
                    float xc = x_start + bx * x_size;
                    float yc = y_start + by * y_size;
                    float h = glm_max(height + srandf(10.0), min_height);
                    float x_size_loc = x_size;

                    if (num_x == 2 && (by > 0 && by != (num_y - 1))) {
                        if (rolldice(city->buildings.backyard_chance)) {
                            float backyard_size = randf(build_area_width / 8.0);
                            if (backyard_size > 0.2) {
                                backyard = true;
                                x_size_loc -= backyard_size;
                                if (!bx) {
                                    xc -= backyard_size / 2.0;
                                } else {
                                    xc += backyard_size / 2.0;
                                }
                            }
                        }

                        float x_var = randf(city->buildings.x_variation);
                        if (!bx) {
                            xc += x_var / 2.0;
                        } else {
                            xc -= x_var / 2.0;
                        }
                        x_size_loc -= x_var;
                    }

                    float y_var = 0;
                    if (by != (num_y - 1)) {
                        y_var = srandf(city->buildings.y_variation);
                        y_delta += y_var / 2.0;
                    } else {
                        y_var = -y_delta;
                        y_delta += (y_var / 2.0);
                    }

                    bool modern = rolldice(city->buildings.modern_chance);
                    if (h > city->buildings.small_height) {
                        h = city->buildings.small_height;
                    }

                    plant_building(world, city, e, xc, yc + y_delta, h, 
                        x_size_loc, y_size + y_var, modern, false);
                    
                    y_delta += y_var / 2.0;
                }
            }
            if (backyard) {
                plant_backyard(world, &block, 0, 0);
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
                        float tx = srandf(block_width - pave_width * 4);
                        float ty = srandf(block_height - pave_width * 4);
                        plant_tree(world, &block, tx, ty);
                    }
                }
            }
        }
    }

    if (rolldice(city->props.chance)) {
        plant_prop(world, &block, -corner_w, 0);
    }
    if (rolldice(city->props.chance)) {
        plant_prop(world, &block, corner_w, 0);
    }
    if (rolldice(city->props.chance)) {
        plant_prop(world, &block, 0, -corner_h);
    }
    if (rolldice(city->props.chance)) {
        plant_prop(world, &block, 0, corner_h);
    }

    /* Lanterns on street corners */
    if (city->lanterns) {
        if (!(x_i % 2) && !(y_i % 2)) {
            plant_lantern(world, &block, corner_w, corner_h);
            plant_lantern(world, &block, corner_w, -corner_h);
            plant_lantern(world, &block, -corner_w, -corner_h);
            plant_lantern(world, &block, -corner_w, corner_h);
        } else if (!((x_i + 1) % 2) && !((y_i + 1) % 2)) {
            plant_lantern(world, &block, corner_w, corner_h);
            plant_lantern(world, &block, corner_w, -corner_h);
            plant_lantern(world, &block, -corner_w, -corner_h);
            plant_lantern(world, &block, -corner_w, corner_h);
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
        int block_width = city->block_width;
        int block_height = city->block_height;
        int building_min_width = city->buildings.min_width;
        int pavement_width = city->pavement_width;
        int road_width = city->road_width;
        float traffic_speed = city->traffic.speed;
        float small_height = city->buildings.small_height;

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
        if (!block_width) {
            city->block_width = block_width = 20;
        }
        if (!block_height) {
            city->block_height = block_height = block_width;
        }
        if (!building_min_width) {
            city->buildings.min_width = 4.0;
        }
        if (!pavement_width) {
            city->pavement_width = 2.0;
        }
        if (!road_width) {
            city->road_width = road_width = 10;
        }
        if (!traffic_speed) {
            traffic_speed = 15.0;
        }
        if (!small_height) {
            city->buildings.small_height = 15.0;
        }

        float width = (float)block_width + (float)road_width;
        float height = (float)block_height + (float)road_width;

        float left = -((float)width * blocks_x) / 2.0f;
        float top = -((float)height * blocks_y) / 2.0f;
        left -= width / 2.0;
        top -= height / 2.0;

        CityBound bound = {
            .width = blocks_x * width, 
            .height = blocks_y * height,
            .left =    left  - width / 2.0,
            .right =   -left - width - width / 2.0,
            .top =     top  - width / 2.0,
            .bottom = -top - width - width / 2.0
        };

        ecs_entity_t streets = ecs_new_w_pair(world, EcsChildOf, e);
        ecs_set_name(world, streets, "Streets");
        ecs_add_pair(world, streets, EcsIsA, CityStreet);
        ecs_set(world, streets, EcsPosition3, {-width, STREETS_HEIGHT / 2.0, -height});
        ecs_set(world, streets, EcsBox, {
            blocks_x * width, STREETS_HEIGHT, blocks_y * height});

        for (int x = 0; x < blocks_x; x ++) {
            for (int y = 0; y < blocks_y; y ++) {
                plant_city_block(world, e, city, x, y,
                    left + width * x, 
                    top + height * y);
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
        ecs_entity_t to_delete = 0;

        if(p[i].x < b[i].left) {
            to_delete = e;
        } else if(p[i].x > b[i].right) {
            to_delete = e;
        } else if(p[i].z < b[i].top) {
            to_delete = e;
        } else if(p[i].z > b[i].bottom) {
            to_delete = e;
        }

        ecs_delete(world, to_delete);
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
        CityBin = ecs_lookup_fullpath(world, "Bin");
        CityHydrant = ecs_lookup_fullpath(world, "Hydrant");
        CityBench = ecs_lookup_fullpath(world, "Bench");
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
