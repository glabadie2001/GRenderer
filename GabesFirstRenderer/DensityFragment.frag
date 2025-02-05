#version 430 core
out vec4 FragColor;

in vec4 vertexColor; // the input variable from the vertex shader (same name and same type)  
in float density;
in vec2 TexCoord;

uniform sampler2D densityTex;

void main()
{
    // Read the density value for this pixel
    float density = texture(densityTex, TexCoord).r;
    
    // Map density to color (example using a simple grayscale)
    FragColor = vec4(density, density, density, 1.0);
} 