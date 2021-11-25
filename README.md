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

- [x] see something
- [x] objects
  - [x] load from obj
  - [x] primitives
- [x] cameras
  - [x] rotate camera using mouse (left click - hold)
  - [x] strafe camera using mouse (middle click - hold)
- [x] MeshModel transformations
  - [x] model transformation
  - [x] world transformation
  - [x] camera transformation
  - [x] window/screen transformation
- [x] bounding box
  - [x] calculation
  - [x] drawing
- [x] normals per vertices
  - [x] calculation
  - [x] drawing
- [x] normals per face
  - [x] calculation
  - [x] drawing
- [x] gui
  - [x] add object
    - [x] add obj MeshModel
    - [x] add primitiveModel
  - [x] add camera
    - [x] add by orth-projection
    - [x] add by perspective projection
    - [x] add by fovy
  - [x] resize view and keep projection aspect ratio
  - [x] switch between objects
- [x] switch between cameras
  - [x] switch between frame of reference
  - [x] resize the renderer
  - [x] step size of transformation
  
- [x] render camera indicator