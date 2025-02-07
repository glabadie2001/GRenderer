#ifndef RENDERER_H
#define RENDERER_H

#include "Scene.h"

class Renderer
{
    float _screenWidth;
    float _screenHeight;

    float _screenX;
    float _screenY;

public:
    void updateWindowPosition(const Scene& scene, float x, float y);
    void updateScreenSize(const Scene& scene, float width, float height);
    void renderLoop(const Scene& scene) const;
    void drawMesh(const Mesh& mesh) const;
    void drawParticleSystem(const ParticleSystem& ps) const;
};

#endif

