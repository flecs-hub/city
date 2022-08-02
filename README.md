# city
Simple procedural city generator. Standalone Flecs project with embedded graphics modules ([live webasm demo](https://flecs.dev/city))

<img width="1312" alt="Screen Shot 2021-12-21 at 2 17 27 PM" src="https://user-images.githubusercontent.com/9919222/147004880-da177044-4cdb-4c1c-aec3-c691c4ebad79.png">

## How to run
Use the following commands to run the demo:

Install bake on macOS/Linux:
```
git clone https://github.com/SanderMertens/bake
bake/setup.sh
```

Install bake on Windows
```
git clone https://github.com/SanderMertens/bake
cd bake
setup
```

or if you already have a bake installation:
```
bake upgrade
```

Then run:
```
bake run flecs-hub/city
```

Have fun!

## How to use
To customize the city generation parameters, change the settings in the etc/assets/scene.plecs file.
