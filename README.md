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

### tasks:

- [ ] Clipping
- [ ] Geometry
  - [ ] uniform material for each model
  - [ ] non-uniform material
- [ ] Scene
  - [ ] light source
    - [x] light class
    - [ ] type, color, position
    - [ ] parallel
    - [ ] point
  - [ ] ambient light
- [ ] Renderer
  - [ ] z-buffer algorithm
  - [ ] flat, gouraud, phong shading user switchable