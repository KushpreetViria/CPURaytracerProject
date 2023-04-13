### Base features complete:
- Intersection tests for spheres, planes, and triangles (meshes).
- Lighting calculated using Phong reflection (Phong specular + Lambertian diffuse).
- Ambient, point, directional, and spot lights.
- Shadow testing.
- Mirror reflection and transmission (simple transparency and refraction).
### Additional feature added:
- Extra geometry: **Cylinders**. These are shown in scene `a` and `b`.
- Also partially completed **transformations**, but only for cylinders. Did not have time to add them to the other geometry.
	- These transformations can be noticed in `a_p1.png` and `a_p2.png`, which shows the same 2 cylinder objects but with different rotation and translations applied.
### Additional Scenes added:
- 2 additional scenes, `a.json` and `b.json`
- All images for each scene can be found under `/scene_outputs`
### Note: this was built and run on Windows 11. 
- To run a specific scene, you can build the Visual Studio project, grab the exe file under `/build/opengl.exe` and run this file in `/src` folder. Doing so you can supply command line arguments.
- Or you can run the project through Visual Studio, and manually supply the command line arguments through project properties.
- Or you can alter line 52 of `raytracer.cpp` and change `fn` to the scene you wish to run:
```cpp
50 void choose_scene(char const* fn) {
51  	if (fn == NULL) {
52  		fn = "c";
53	    	std::cout << "Using default input file " << PATH << fn << ".json\n";
54 }
```