#version 430 core
layout (location = 0) in vec2 aPos; // Changed from vec3 to vec2
layout (location = 1) in float aDen;
layout (location = 2) in float aCell;
layout (location = 3) in vec2 aVel;

uniform vec2 screenSize;

out vec4 vertexColor;
out float density;
void main()
{
    vec2 scaledPos = aPos / screenSize;
    // Convert screen coordinates to clip space (-1 to 1)
    vec2 clipPos = scaledPos * 2.0 - 1.0;
    
    gl_Position = vec4(clipPos, 0.0, 1.0);
    gl_PointSize = 10.0;
    
    density = aDen;
    vertexColor = vec4(0.0, 0.0, aCell/10000.0, 1.0);
    //vertexColor = vec4(scaledPos, 0.0, 1.0);
    //float spd = sqrt(aVel.x * aVel.x + aVel.y * aVel.y) / 1000.0;
    //vertexColor = vec4(spd, 0, 0, 1);
}