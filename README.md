## download gltf data
https://sketchfab.com/3d-models/girl-warrior-motorcycle-cyber-by-oscar-creativo-c27030b41da04de29b0ba78c486a9b31#download

## For
The descriptor set number 0 will be used for engine-global resources, and bound once per frame. // camera
The descriptor set number 1 will be used for material resources, 
and the number 2 will be used for per-object resources. 
This way, the inner render loops will only be binding descriptor sets 1 and 2, and performance will be high.

## TODO List
- [ ] basic BRDF
- [ ] [normal maping](https://learnopengl.com/Advanced-Lighting/Normal-Mapping)
- [ ] [parallax occlusion maping](https://learnopengl.com/Advanced-Lighting/Parallax-Mapping)
- [ ] principle BSDF
- [ ] subsurface scattering