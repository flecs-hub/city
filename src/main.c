#include <city.h>

int main(int argc, char *argv[]) {    
    ecs_world_t *world = ecs_init();

    ECS_IMPORT(world, FlecsUnits);
    ECS_IMPORT(world, FlecsScript);
    ECS_IMPORT(world, FlecsComponentsTransform);
    ECS_IMPORT(world, FlecsComponentsGeometry);
    ECS_IMPORT(world, FlecsComponentsGui);
    ECS_IMPORT(world, FlecsComponentsGraphics);
    
    ECS_IMPORT(world, FlecsGame);
    ECS_IMPORT(world, FlecsCity);

    ecs_time_t t = {0}; ecs_time_measure(&t);
    ecs_script_run_file(world, "etc/assets/scene.flecs");
    printf("scene loaded in %fs\n", ecs_time_measure(&t));

    /* Prewarm simulation so there's cars everywhere */
    for (int i = 0; i < 5000; i ++) {
        ecs_progress(world, 0.16);
    }

    ECS_IMPORT(world, FlecsSystemsTransform);
    ECS_IMPORT(world, FlecsSystemsSokol);
    ecs_script_run_file(world, "etc/assets/app.flecs");

    return ecs_app_run(world, &(ecs_app_desc_t){
        .enable_rest = true,
        .enable_stats = true,
        .target_fps = 60
    });
}
