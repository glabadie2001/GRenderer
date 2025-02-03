#version 330 core
layout (location = 0) in vec2 aPos; // Changed from vec3 to vec2
layout (location = 1) in float aDen;

uniform vec2 screenSize;

out vec4 vertexColor;
out float density;
void main()
{
    // Convert screen coordinates to clip space (-1 to 1)
    vec2 clipPos = (aPos / screenSize) * 2.0 - 1.0;
    
    gl_Position = vec4(clipPos, 0.0, 1.0);
    gl_PointSize = 10.0;
    
    density = aDen;
    vertexColor = vec4(0.0, 0.0, aDen, 1.0);
}