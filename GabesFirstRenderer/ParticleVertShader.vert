#version 430 core
layout (location = 0) in vec2 aPos; // Changed from vec3 to vec2
layout (location = 1) in float aDen;
layout (location = 2) in float aCell;
layout (location = 3) in vec2 aVel;

uniform vec2 screenSize;
uniform float targetDensity;
uniform mat4 projection;  // Orthographic projection matrix

out vec4 vertexColor;
out float density;

void main()
{
    //vec2 scaledPos = aPos / screenSize;
    // Convert screen coordinates to clip space (-1 to 1)
    //vec2 clipPos = scaledPos * 2.0 - 1.0;
    
    //gl_Position = vec4(clipPos, 0.0, 1.0);
    
    gl_Position = projection * vec4(aPos, 0.0, 1.0);
    gl_PointSize = 5.0;
    
    density = aDen;
    //vertexColor = vec4(0.0, 0.0, aCell / 5000.0, 1.0);
    vertexColor = vec4(0.0, aDen / targetDensity, 0.0, 1.0);
    //vertexColor = vec4(aVel.x/800.0, aVel.y/600.0, 0, 1.0);
    //float spd = sqrt(aVel.x * aVel.x + aVel.y * aVel.y) / 1000.0;
    //vertexColor = vec4(spd, 0, 0, 1);
}