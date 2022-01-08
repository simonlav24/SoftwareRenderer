# OpenGL Cheat Sheet

* glUseProgram(GLuint program);
* rendering data:
  glDrawArrays(GL_TRIANGLES,0,sizeof(points));

### Vertex Array Object - VAO

saves vertexes and more (texture coordinaets ...)

```c++
GLuint vao;
glGenVertexArrays(1, &vao); // how much and where to save
glBindVertexArray(vao); // vao is the current active 
```

### Vertex Buffer Object - VBO

```c++
GLuint buffer;
glGenBuffers(1, &buffer);
glBindBuffer(GL_ARRAY_BUFFER, buffer);
glBufferData(GL_ARRAY_BUFFER,sizeof(points), points, GL_STATIC_DRAW);
```

#### Vertex Shader

input: vertex location (object frame)

output: vertex location to the rasterizer

#### Fragment Shader

input: fragment (pixel, depth)

output: color of the fragment

### PoolProccess:

glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

set camera transform

### GLSL

in -> input into the shader (copied each time)

uniform -> input that copied once. for things (mats, ...) that do not change in the action

out ->

#### CPP

```c++
/// השמה ליוניפורם
GLint modelviewLoc = glGetUniformLocation(program, "modelview");// defining modelview as uniform 
glUniformMatrix4fv(modelviewLoc, 1/*one matrix*/, GL_FALSE/*do not transpose*/, mvMat/*the actual matrix in cpp*/);
```





