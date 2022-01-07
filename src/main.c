#include <city.h>

int main(int argc, char *argv[]) {
    ecs_log_set_level(0);
    
    ecs_world_t *world = ecs_init();
    
    ECS_IMPORT(world, FlecsCity);
    ECS_IMPORT(world, FlecsGame);
    ECS_IMPORT(world, FlecsComponentsTransform);
    ECS_IMPORT(world, FlecsComponentsGeometry);
    ECS_IMPORT(world, FlecsComponentsGui);
    ECS_IMPORT(world, FlecsComponentsGraphics);
    ECS_IMPORT(world, FlecsSystemsSokol);

    ecs_plecs_from_file(world, "etc/assets/app.plecs");

    ecs_time_t t = {0}; ecs_time_measure(&t);
    ecs_plecs_from_file(world, "etc/assets/scene.plecs");
    printf("scene loaded in %fs\n", ecs_time_measure(&t));

    return ecs_app_run(world, &(ecs_app_desc_t) {
        .target_fps = 60, .enable_rest = true
    });
}
