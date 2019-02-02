# Redline - C++ Path Tracer

Redline is a path tracer written in C++ as a educational and hobby project. It takes [GLTF 2.0](https://www.khronos.org/gltf/) scene files as a input and renders them with a unbaised path tracer. The software itself is not particularly practicle to use in it's current state, but the source code may be useful to somebody else trying to learn about offline rendering. It currently only builds for Windows, but has been written with cross platform compatibility in mind (SDL for window management).
## Built With

* [MathFu](https://github.com/google/mathfu) - Vector math library
* [ObjLoader](https://github.com/Bly7/OBJ-Loader) - Obj mesh loader
* [SDL2](https://www.libsdl.org/) - Window management and display
* [JSON for Modern C++](https://github.com/nlohmann/json) - JSON Parser
* [STB Image](https://github.com/nothings/stb) - Image loader
* [LodePNG](https://lodev.org/lodepng/) - PNG loader
* [Tiny GLTF](https://github.com/syoyo/tinygltf) - GLTF 2.0 scene loader

## Authors

* **Jonathan Linsner** - *Initial work* - [64-Bit on Github](https://github.com/64-bit)

## License

This project is licensed under the MIT License - see the [LICENSE.md](LICENSE.md) file for details

## Acknowledgments

Big thanks to the following authors, they where instrumental in developing my knowledge of path tracing, PBR and importance sampling.

#### Blogs
* Joe Schutte - https://schuttejoe.github.io/
* Marco Alamia - http://www.codinglabs.net/default.aspx
* Cao Jiayin - https://agraphicsguy.wordpress.com/
#### Papers

* Brian Karis - [Real Shading in Unreal Engine 4
](https://cdn2.unrealengine.com/Resources/files/2013SiggraphPresentationsNotes-26915738.pdf)
* Earl Hammon, Jr. - [PBR Diffuse Lighting for GGX+Smith Microsurfaces](https://twvideo01.ubm-us.net/o1/vault/gdc2017/Presentations/Hammon_Earl_PBR_Diffuse_Lighting.pdf)

## Renders

Here are a few test renders from the development process

![Image](https://i.imgur.com/cgFmz1q.jpg)
![Imgur](https://i.imgur.com/dzyl0Al.jpg)