# Generative Computer Graphics Exercises

A package of exercises for the course in Generative Methods in Computer Graphics.

The exercises are based on two executables:

```
./bin/implicits
./bin/pool_scene
```

All work on the exercises involves changing shaders written in the _GLSL shading language_. The programs search for the shader files relative to `bin` directory which contains the programs. Thus, one must run the programs _from_ the `bin` directory. Note that prebuilt versions of the programs are in `bin.mac` for MacOS and `bin.win` for Windows.

## Interaction

The two programs support interaction via mouse and keyboard. 

For the `implicits` program, there are the following interactive controls:
- Press **R** to make the program reload shaders. This is, of course, necessary during the development process.
- Arrow up and down move the camera closer and farther away. **F** and **c** cause the FOV to change which has roughly the same effect.
- Left mouse button rotates the camera, and right mouse button pans the camera. Middle button also pans.
- **ESC** quits the application.

The `poolscene` program has the following interactive controls:
- **WASD** moves in the usual FPS fashion, and the mouse looks around - again as you would expect in a first person game.
- Press **R** to make the program reload shaders.
- **ESC** quits the application.

## Exercises in this package

`implicits` is a program that renders implicit surfaces by ray casting in a fragment shader. Out of the box, this program shows just a single sphere, and the task is to create a more complex scene by only modifying the fragment shader `shaders/Implicits/object.frag`.

`pool_scene` is a program that renders a very simple scene with a pool of water. The shader code for this program is in the directory `shaders/PoolScene`.
There are several exercises associated with this program:
- A spray of water spouts from one of the lion heads when the correct code has been filled into the appropriate shaders. This work is carried out in the `particles.vert` and `particles.geom`.
- Fish swim around in accordance with a simple _boids_ implementation. The code to make this happen should be injected in `fish.vert` and `update_fish.frag`.
- There are also ripples in the pool. To make this work, you need to edit `fluid_solver.frag` and fix the rendering in `render_water.frag`.
- Finally, there is an exercise about parallax mapping which can be added to the walls. To make parallax mapping work, you need to edit `room.frag`.

## Running the binary versions

The two programs have been compiled for MacOS and Windows. It is not trivial to ensure that the programs run on all machines since it depends on the precise OS, machine architecture, and installed dynamic libraries. With this caveat, you should try the binaries before compiling if you have an Apple Silicon Mac or a Windows/Intel computer.

### MacOS 

The programs have been compiled on MacOS Ventura and tested on and M1 and M2 machine. The binaries should work (possibly also on earlier versions of MacOS) if the machine has Apple Silicon. However, you may need to remove a quarantine bit from the programs. This is done as follows.

```
xattr -d com.apple.quarantine ./bin/implicits
xattr -d com.apple.quarantine ./bin/pool_scene
```
## Building

There are a few prerequisites for building the programs:

1. OpenGL must be installed for these programs to work. The programs also rely on GLFW, but this package is installed as part of the build process.
2. CMake must be installed in a version > 3.10.
3. You need the standard toolchain: C++ compiler, linker, and make.

### Unix
To build and install in a UNIX-like environment (Linux or MacOS), enter the following commands from a shell whose current directory as where this README is located:

```
cd build ; cmake .. ; make -j 10 install ; cd ..
```
### Windows
If you are on a Windows machine, you can run CMake as an interactive program and then open `GCG.sln` in Visual Studio, or you could run both `cmake` and `msbuild` from a PowerShell command line.


