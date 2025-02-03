#version 330 core
out vec4 FragColor;

in vec4 vertexColor; // the input variable from the vertex shader (same name and same type)  
in float density;

void main()
{
    float scalar = 10.0f;
    //FragColor = vec4(density / scalar, density / scalar, density / scalar, 1.0f);
    FragColor = vertexColor;
} 