#ifndef BRAIN_COMPONENT_H
#define BRAIN_COMPONENT_H


#include "Component.h"
// third party include
#include <glm/glm.hpp>
#include <list>

// forward declerations
class Entity;

class BrainComponent : public Component
{
public:
	BrainComponent(Entity* a_pEntity);

	// functions for doing processes each frame and rendering
	virtual void Update(float a_fDeltaTime, float a_fBoundingBoxSize);
	virtual void Draw(Shader* a_pShader) {}

	// returns velocity of the boid
	glm::vec3 GetCurrentVelocity() const { return m_v3CurrentVelocity; }
	// updates the forces for the boid
	void UpdateForces(float a_fDeltaTime);

	// values that are edited by the gui
	float wanderWeight;
	float cohesionWeight;
	float separationWeight;
	float allignmentWeight;
	glm::vec3 boxPos;

private:
	// functions for calculating the behaviour forces of the boids
	glm::vec3 CalculateForces();
	glm::vec3 CalculateSeekForce(const glm::vec3& v3Target, const glm::vec3& v3CurrentPos) const;
	glm::vec3 CalculateFleeForce(const glm::vec3& v3Target, const glm::vec3& v3CurrentPos) const;
	glm::vec3 AvoidBox(const glm::vec3& v3Target, const glm::vec3& v3CurrentPos) const;
	glm::vec3 CalculateWanderForce(const glm::vec3& v3Forward, const glm::vec3& v3CurrentPos);

	// Flocking behaviours
	glm::vec3 CalculateSeparationForce(glm::vec3 v3SeparationVel, unsigned int uNeighbourCount);
	glm::vec3 CalculateAlignmentForce(glm::vec3 v3AllignmentVel, unsigned int uNeighbourCount);
	glm::vec3 CalculateCohesionForce(glm::vec3 v3LocalPos, glm::vec3 v3CohesionVel, unsigned int uNeighbourCount);

	glm::vec3 UpdateBoundsFleeForce(float a_fBoundsSize, glm::vec3 a_v3LocalPos);

	// Var
	glm::vec3 m_v3CurrentVelocity;
	glm::vec3 m_v3WanderPoint;
	glm::vec3 m_v3UpperVelClamp = glm::vec3(2.5f, 2.5f, 2.5f);
	glm::vec3 m_v3LowerVelClamp = glm::vec3(-2.5f, -2.5f, -2.5f);

	float fMaxSpeed = 1.0f;

	bool bCalculateThisLoop = true;

	float boxSize = 0.5f;
	float boxRadius = 1.0f;

};

#endif // !BRAIN_COMPONENT_H