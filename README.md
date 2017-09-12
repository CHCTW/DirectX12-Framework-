# DirectX12-Framework-
A framework for learning DirectX12 and demonstrate graphics techniques 
![image](https://user-images.githubusercontent.com/30221942/30350698-b44b46da-97cc-11e7-86af-c56de405dbae.png)

## All Unitest and Graphic Techniques pojects files working space is incorrect. Right click the project file and select properties.Click Debugging and find Working Directory area. Change it to $(SolutionDir) .

## The framework sperate into sveral parts:

### 1. Unit Tests: This part has a lot of little projects to test or demostrate the basic funcionality of this framework. The contents are:

1. Triangle: Test basic vertex drawing and use texture 
2. Constant Buffer : Use constant buffer and change the value of constant buffer in every frame to move Triangle
3. Index Buffer : Use Index draw to draw two triangle
4. Structered Buffer : Use structered buffer and instanced_ID in HLSL to draw four triangle in one draw call
5. Structered Buffer Write : Use structered buffer and compute shader to update the contents of structer buffer
6. AppendConsumeBuffer : Use coounter for buffer that allow HLSL use append/consume in shader code
7. Depth Test: Draw two triangle with reverse depth order and use depth as their color to make sure depth buffer work correctlly
8. FrameBuffer: Draw the triagnel in the framebuffer and use this framebuffer as a texture of the triangle and draw it again
9. Model Loading : Use assimp to load and draw obj file
10. Gamma Correction : Test for gamma correction
11. Skybox : Test TextureCube
12. Root Constants : Drw four triangles with four draw call and use root constant to decide the position and color


### 2. Graphics Techniques: Projects in this part is for demostaring some graphcis techniques(lighting, deffered shading...)

BRDF : Use Physics Basic Rendering to lit the scene and demostrate how different material looks like
Shadow Map : Use one simple shadow map to demostrate the shadow for spot light
OmniDirection Shadow Map : use cube mapping to demostrate the shadow for point light

### 3. Direcx12 Framework : This project is included by all the Unitests and Graphics Techniques. It is the core of this framework. It only generate lib file.
### 3. Graphics Utility : This project is included by part Unitests and Graphics Techniques. It contains some helper structer like spot light, Transform, Camera.... It only generate lib file.
