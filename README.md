# DirectX12-Framework-
A framework for learning DirectX12 and demonstrate graphics techniques 
![image](https://user-images.githubusercontent.com/30221942/34930601-1682d296-f97f-11e7-9211-8c52055a1d42.png)

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
https://www.youtube.com/watch?v=uF2_494RuAI
![image](https://user-images.githubusercontent.com/30221942/31530941-9f3a47a8-af98-11e7-9cc3-59843aa5a325.png)
16. GeometryShader: Take normal model, for each triagnel do some movement base on time and generate a small spike for each triangle
https://www.youtube.com/watch?v=IpE9jaEErks
![image](https://user-images.githubusercontent.com/30221942/31573775-273d989c-b077-11e7-9298-bd2c6c52d0fd.png)
17. BezierPlane : Use Perlin noise to generate different control points and use Bezier curve and tessellation to generate more detail information. The tessellation factor is based on the distance to the camera.https://www.youtube.com/watch?v=HxNtCPt5gPw&feature=youtu.be
![image](https://user-images.githubusercontent.com/30221942/31639809-9b22bff2-b28f-11e7-8f40-5b57fb8cab29.png)
18. DispalcementMap : Use height map and tessellation shader to add more LOD on Models.The textures is from : https://freepbr.com
https://www.youtube.com/watch?v=dcNFsgGAZaM
![image](https://user-images.githubusercontent.com/30221942/31986129-1bef2dea-b91c-11e7-8e8b-5a2790f47bc6.png)
19. Execute Indirect : Using execute indirect drawing 8000 spheres(1984 triangles for each sphere). Also using frustum culling to cull out the spheres that will not  show in the camera frustum. https://www.youtube.com/watch?v=9GI294MPXeA&t=2s&index=10&list=PLvLvpclrF-V2Mdev5MhF7zl5RDFbaqnBF
![image](https://user-images.githubusercontent.com/30221942/32411080-10b9a7e0-c18f-11e7-8d76-2a855768f004.png)
20. OcculusionQuery : An add up from unitest-19. Should be using query heap to query occulusion. But setPrediction works terriblly with execute indirect. Change to pixel shader approach to finish it. There will be disorder effect when camera moving to fast. This probally can be sovled by using bigger box? or other method. Can use more low resolution depth-buffer to gain more performance.

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
![image](https://user-images.githubusercontent.com/30221942/30882868-408c5a74-a2bf-11e7-98c6-b360ee58812c.png)
Multiple Point Lights ShadowMap : Based on the method :  Tile-Based Omnidirectional Shadows by Hawar Doghramachi. Which can be found in GPU Pro6. http://gpupro.blogspot.com/2014/12/gp...
The whole scene consist with 128 point lights. The fame time is about 70ms.
I use cube shadow map but make the whole pass with execute indirect to largely decrease the draw commands. 
Also I am adding the bloom effect. The bloom effect is learned from dx12 mini engine.
https://www.youtube.com/watch?v=YpxZGnVfkb4&feature=youtu.be
![image](https://user-images.githubusercontent.com/30221942/34026871-cfa18ebe-e10d-11e7-8080-4effde1b179d.png)
Cascaded shadow map : Use stable cascaded shadow map to largely add shadow quality, also combined with SMSR to decrease aliasing of shadows. SMSR can be found in :GPU Pro6: http://gpupro.blogspot.com 
This is directional light version. Each shadow maps resolution is 1024*1024
https://www.youtube.com/watch?v=plOWDfTVvpE&t=16s&list=PLvLvpclrF-V2Mdev5MhF7zl5RDFbaqnBF&index=12
![image](https://user-images.githubusercontent.com/30221942/34930601-1682d296-f97f-11e7-9211-8c52055a1d42.png)


### 3. Direcx12 Framework : This project is included by all the Unitests and Graphics Techniques. It is the core of this framework. It only generate lib file.
### 4. Graphics Utility : This project is included by part Unitests and Graphics Techniques. It contains some helper structer like spot light, Transform, Camera.... It only generate lib file.
