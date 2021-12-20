#include "flecs_systems_transform.h"

void EcsAddTransform3(ecs_iter_t *it) {
    ecs_world_t *world = it->world;
    ecs_entity_t comp = ecs_term_id(it, 1);

    int i;
    for (i = 0; i < it->count; i ++) {
        ecs_add_id(world, it->entities[i], comp);
    }
}

void EcsApplyTransform3(ecs_iter_t *it) {
    EcsTransform3 *m = ecs_term(it, EcsTransform3, 1);
    EcsTransform3 *m_parent = ecs_term(it, EcsTransform3, 2);
    EcsPosition3 *p = ecs_term(it, EcsPosition3, 3);
    EcsRotation3 *r = ecs_term(it, EcsRotation3, 4);
    EcsScale3 *s = ecs_term(it, EcsScale3, 5);
    int i;

    if (!m_parent) {
        if (ecs_term_is_owned(it, 3)) {
            for (i = 0; i < it->count; i ++) {
                glm_translate_make(m[i].value, *(vec3*)&p[i]);
            }
        } else {
            for (i = 0; i < it->count; i ++) {
                glm_translate_make(m[i].value, *(vec3*)p);
            }
        }
    } else {
        if (ecs_term_is_owned(it, 3)) {
            for (i = 0; i < it->count; i ++) {
                glm_translate_to(m_parent[0].value, *(vec3*)&p[i], m[i].value);
            }
        } else {
            for (i = 0; i < it->count; i ++) {
                glm_translate_to(m_parent[0].value, *(vec3*)p, m[i].value);
            }
        }
    }

    if (r) {
        for (i = 0; i < it->count; i ++) {
            glm_rotate(m[i].value, r[i].x, (vec3){1.0, 0.0, 0.0});
            glm_rotate(m[i].value, r[i].y, (vec3){0.0, 1.0, 0.0});
            glm_rotate(m[i].value, r[i].z, (vec3){0.0, 0.0, 1.0});
        }
    }

    if (s) {
        for (i = 0; i < it->count; i ++) {
            glm_scale(m[i].value, *(vec3*)&s[i]);
        }
    }
}

void FlecsSystemsTransformImport(
    ecs_world_t *world)
{
    ECS_MODULE(world, FlecsSystemsTransform);
    ECS_IMPORT(world, FlecsComponentsTransform);

    ecs_set_name_prefix(world, "Ecs");

    /* System that adds transform matrix to every entity with transformations */
    ECS_SYSTEM(world, EcsAddTransform3, EcsPostLoad,
        [out] !flecs.components.transform.Transform3,
        flecs.components.transform.Position3(self|super) || 
        flecs.components.transform.Rotation3(self|super) || 
        flecs.components.transform.Scale3(self|super));

    ECS_SYSTEM(world, EcsApplyTransform3, EcsOnValidate, 
        flecs.components.transform.Transform3,
        ?flecs.components.transform.Transform3(parent|cascade),
        flecs.components.transform.Position3(self|super),
        ?flecs.components.transform.Rotation3,
        ?flecs.components.transform.Scale3);
}

