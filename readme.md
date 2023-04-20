### Note: this was built and run on Windows 11. 
- To run a specific scene, you can build the Visual Studio project, grab the exe file under `/build/opengl.exe` and run this file in `/src` folder. Doing so you can supply command line arguments.
- Or you can run the project through Visual Studio, and manually supply the command line arguments through project properties.
- Or you can alter line 52 of `raytracer.cpp` and change `fn` to the scene you wish to run, then run the program with no command line arguments:
```cpp
50 void choose_scene(char const* fn) {
51  	if (fn == NULL) {
52  		fn = "c";
53	    	std::cout << "Using default input file " << PATH << fn << ".json\n";
54 }
```