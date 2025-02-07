#include "Renderer.h"
#include "utils.h"
void Renderer::updateScreenSize(const Scene& scene, float width, float height) {
	_screenWidth = width;
	_screenHeight = height;

	std::vector<ParticleSystem*> psList = scene.getParticleSystems();

	int i;
	for (i = 0; i < psList.size(); i++) {
		psList[i]->updateScreenSize(width, height);
	}
}

void Renderer::updateWindowPosition(const Scene& scene, float x, float y) {
	_screenX = x;
	_screenY = y;
	std::vector<ParticleSystem*> psList = scene.getParticleSystems();

	int i;
	for (i = 0; i < psList.size(); i++) {
		psList[i]->setWindowPosition(x, y);
	}
}

void Renderer::renderLoop(const Scene& scene) const
{
	int i;

	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	std::vector<Mesh*> meshes = scene.getMeshes();

	for (i = 0; i < meshes.size(); i++) {
		drawMesh(*meshes[i]);
	}

	std::vector<ParticleSystem*> psList = scene.getParticleSystems();

	for (i = 0; i < psList.size(); i++) {
		drawParticleSystem(*psList[i]);
	}
}

void Renderer::drawMesh(const Mesh& mesh) const {
	mesh.shader->use();

	glBindVertexArray(mesh.getVertices());
	if (mesh.getIndCount() > 0) {
		glDrawElements(GL_TRIANGLES, mesh.getIndCount(), GL_UNSIGNED_INT, 0);
	}
	else {
		glDrawArrays(GL_TRIANGLES, 0, mesh.getVertCount());
	}
	glBindVertexArray(0);
}

void Renderer::drawParticleSystem(const ParticleSystem& ps) const {
	ps.shader->use();

	ps.shader->setFloat("targetDensity", ps.getTargetDensity());
	ps.shader->setVec2("screenSize", glm::vec2(800, 600));

	glBindVertexArray(ps.getVertices());

	// If you need to update the positions:
	glBindBuffer(GL_ARRAY_BUFFER, ps.getVBO());
	glBufferSubData(GL_ARRAY_BUFFER, 0, ps.count() * sizeof(glm::vec2), ps.positions);

	glBindBuffer(GL_ARRAY_BUFFER, ps.getDensityBuff());
	glBufferSubData(GL_ARRAY_BUFFER, 0, ps.count() * sizeof(float), ps.densities);

	glEnable(GL_PROGRAM_POINT_SIZE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Draw the points
	glDrawArrays(GL_POINTS, 0, ps.count());

	// 6. Check for errors
	printErrors();

	glDisable(GL_PROGRAM_POINT_SIZE);
	glDisable(GL_BLEND);
	glBindVertexArray(0);
}
