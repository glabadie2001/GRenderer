#ifndef RENDERER_H
#define RENDERER_H

#include "Scene.h"

class Renderer
{
public:
    void renderLoop(const Scene& scene) const;
    void drawMesh(const Mesh& mesh) const;
    void drawParticleSystem(const ParticleSystem& ps) const;
};

#endif

