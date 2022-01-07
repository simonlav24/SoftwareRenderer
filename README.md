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
* **L** - toggle super sampling anti aliasing
* **K** - toggle light bloom

### tasks:

- [x] see some models
- [ ] light shadings
  - [x] flat
  - [x] gouraud
  - [ ] phong
- [ ] light types
  - [ ] point
  - [ ] parallel
- [ ] draw all indicators
  - [ ] lights
  - [ ] bounding box
  - [ ] normals face
  - [ ] normals vertex
- [ ] in line shaders, can send the multiplied matrix
- [ ] problem with gouraud with some models: shark, sphere

### for lighting shader I need:

material: emissive, diffuse, ambient, specular, shininess

model (in): vertex_positions, face_normals, vertex_normals

matrices (uniform):  model, world, ctransform, proj

lights: 

