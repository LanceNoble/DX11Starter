I made the textures scroll along the meshes.
I did this by adding a float uvOffset to the pixel shader's cbuffer
then I created a static float uvOffset in the Entity class' draw method that added 0.00005f per call
then I passed this value via pixelShader->SetFloat();