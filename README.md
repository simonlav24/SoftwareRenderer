# 3D Software Renderer

### User Guide

**mouse**:

* **left click hold** - rotate camera
* **middle click hold** - translate camera
* **right click** - open editing menu

**keys**

* **W** - translate/scale/rotate around positive z axis
* **A** - translate/scale/rotate around positive x axis
* **S** - translate/scale/rotate around negative z axis
* **D** - translate/scale/rotate around negative x axis
* **Q** - translate/scale/rotate around positive Y axis
* **Z** - translate/scale/rotate around negative Y axis
* **+** - increase incremental step size
* **-** - decrease incremental step size
* **1** - switch transformation mode to Position
* **2** - switch transformation mode to Scale
* **3** - switch transformation mode to Rotation
* **M** - cycle switch active model
* **N** - toggle show face normals
* **B** - toggle show bounding box
* **V** - toggle show vertex normals
* **G** - toggle show grid
* **7-8** - control far point of fog
* **9-0** - control near point of fog

### tasks:

- [ ] Clipping
- [x] user input from website
- [x] Material
  - [x] uniform material for each model
  - [x] non-uniform material (iridiscent)
- [ ] Scene
  - [ ] light source
    - [x] light class
    - [x] color of light
    - [x] parallel
    - [x] point
    - [ ] ambient light source
- [x] Renderer
  - [x] different color for ambient, diffuse, specular
  - [x] z-buffer
  - [x] flat, gouraud, phong shading user switchable
    - [x] flat
    - [x] gouraud
    - [x] phong
    - [x] wireframe
- [ ] fix vertex normals like in face normal
- [x] fix orthogonal camera backface culling
- [ ] gaussian blur light bloom
- [x] fog (menu and controling far, near)
- [x] super sampling anti aliasing (menu)

