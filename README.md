# DirectX12-Framework-
A framework for learning DirectX12 and demonstrate graphics techniques 
![image](https://user-images.githubusercontent.com/30221942/30778454-2ceaa09c-a08b-11e7-9b5c-1215392cc16b.png)

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
12. Root Constants : Draw four triangles with four draw call and use root constant to decide the position and color
13. MipmapGenerate : Generate mipmap in CPU
14. IrradianceCubeMap : Take a HDR Cube map and generate Irradiance Cube Map
15. GPUParticles : Use Compute shaders to generate and update 1000000 particles

### 2. Graphics Techniques: Projects in this part is for demostaring some graphcis techniques(lighting, deffered shading...)

BRDF : Use Physics Basic Rendering to lit the scene and demostrate how different material looks like
![image](https://user-images.githubusercontent.com/30221942/30778441-e20eee2a-a08a-11e7-8c28-bbb204175746.png)
Shadow Map : Use one simple shadow map to demostrate the shadow for spot light
![image](https://user-images.githubusercontent.com/30221942/30778472-b55a28e4-a08b-11e7-837f-89b9eabced1b.png)
OmniDirection Shadow Map : use cube mapping to demostrate the shadow for point light
![image](https://user-images.githubusercontent.com/30221942/30778465-6dd2e682-a08b-11e7-932d-416469c74cad.png)
Deferred Shading : Draw all parameters that is need for lighting caculation to two render targets first. In next pass, calculate the lighting result by accessing the data from two render targets and depth buffer and draw on the screen. This demo aslo desmotrate use lighting volume can draw large number of lights in the scene.
![image](https://user-images.githubusercontent.com/30221942/30778454-2ceaa09c-a08b-11e7-9b5c-1215392cc16b.png)
Imaged Based Lighting : Use enviroment map as light source. The method I used is from : https://learnopengl.com/#!PBR/IBL/Diffuse-irradiance



### 3. Direcx12 Framework : This project is included by all the Unitests and Graphics Techniques. It is the core of this framework. It only generate lib file.
### 4. Graphics Utility : This project is included by part Unitests and Graphics Techniques. It contains some helper structer like spot light, Transform, Camera.... It only generate lib file.
