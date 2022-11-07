#include "BrainComponent.h"

// Project headers
#include "Entity.h"
#include "TransformComponent.h"

// constants
static const float fSPEED = 0.1f;
static const float fNEIGHBOURHOOD_RADIUS = 5.0f;
// wander constant
static const float fCIRCLE_FORWARD_MULTIPLIER = 1.0f;
static const float fJITTER = 0.5f;
static const float fWANDER_RADIUS = 4.0f;

BrainComponent::BrainComponent(Entity* a_pOwner) : Component(a_pOwner), m_v3CurrentVelocity(0.0f), m_v3WanderPoint(0.0f)
{
	m_eComponentType = BRAIN;
}

void BrainComponent::Update(float a_fDeltaTime, float a_fBoundingBoxSize)
{
	// Get entity owner
	Entity* pEntity = GetOwnerEntity();
	if (!pEntity)
	{
		return; // early out
	}

	// Get transform comp
	TransformComponent* pTransComp = static_cast<TransformComponent*>(pEntity->FindComponentOfType(TRANSFORM));
	if (!pTransComp)
	{
		return; // early out
	}

	glm::vec3 v3Forward = pTransComp->GetEntityMatrixRow(FORWARD_VECTOR);
	glm::vec3 v3CurrentPos = pTransComp->GetEntityMatrixRow(POSITION_VECTOR);

	// Apply force
	m_v3CurrentVelocity += UpdateBoundsFleeForce(a_fBoundingBoxSize, v3CurrentPos);
	m_v3CurrentVelocity += AvoidBox(boxPos, v3CurrentPos);

	// Clamp Vel
	glm::vec3 m_v3MaxVel = glm::vec3(0.02f * fMaxSpeed);
	m_v3CurrentVelocity = glm::clamp(m_v3CurrentVelocity, -m_v3MaxVel, m_v3MaxVel);

	// Apply vel to position
	v3CurrentPos += m_v3CurrentVelocity;

	// Get our new forward and normalise
	v3Forward = m_v3CurrentVelocity * a_fDeltaTime;

	if (glm::length(v3Forward) > 0.0f)
	{
		v3Forward = glm::normalize(v3Forward);
	}

	// Up and right
	glm::vec3 v3Up = pTransComp->GetEntityMatrixRow(UP_VECTOR);

	// orthonormalisation
	v3Up = v3Up - (v3Forward * glm::dot(v3Forward, v3Up));
	if (glm::length(v3Up) > 0.0f)
	{
		v3Up = glm::normalize(v3Up);
	}

	glm::vec3 v3Right = glm::cross(v3Up, v3Forward);
	if (glm::length(v3Right) > 0.0f)
	{
		v3Right = glm::normalize(v3Right);
	}

	// Update matrix
	pTransComp->SetEntityMatrixRow(RIGHT_VECTOR, v3Right);
	pTransComp->SetEntityMatrixRow(FORWARD_VECTOR, v3Forward);
	pTransComp->SetEntityMatrixRow(UP_VECTOR, v3Up);
	pTransComp->SetEntityMatrixRow(POSITION_VECTOR, v3CurrentPos);

}

void BrainComponent::UpdateForces(float a_fDeltaTime)
{
	// get owner entity
	Entity* pEntity = GetOwnerEntity();
	if (!pEntity) return;
	
	// get transform component
	TransformComponent* pTransComp = static_cast<TransformComponent*>(pEntity->FindComponentOfType(TRANSFORM));
	if (!pTransComp) return;

	// get vectors for calculations
	glm::vec3 v3Forward = pTransComp->GetEntityMatrixRow(FORWARD_VECTOR);
	glm::vec3 v3CurrentPos = pTransComp->GetEntityMatrixRow(POSITION_VECTOR);

	// calculate force
	glm::vec3 v3FinalForce = CalculateForces();
	v3FinalForce += CalculateWanderForce(v3Forward, v3CurrentPos);

	m_v3CurrentVelocity += v3FinalForce * (a_fDeltaTime / 2);
}


glm::vec3 BrainComponent::CalculateForces()
{
	// Final allignment force
	glm::vec3 v3AllignmentVel(0.0f);
	// Final separation vector
	glm::vec3 v3SeparationVel(0.0f);
	// Final Cohesion vector
	glm::vec3 v3CohesionVel(0.0f);

	unsigned int uNeighbourCount = 0;

	// Get this entities transform (should be a const but it doesnt work)
	TransformComponent* pLocalTransform = static_cast<TransformComponent*>(GetOwnerEntity()->FindComponentOfType(TRANSFORM));
	if (!pLocalTransform)
	{
		return glm::vec3(0); // early out
	}

	// Get this entities transform values
	glm::vec3 v3LocalPos = pLocalTransform->GetEntityMatrixRow(POSITION_VECTOR);
	glm::vec3 v3Forward = pLocalTransform->GetEntityMatrixRow(FORWARD_VECTOR);

	// create an iterator
	const std::map<const unsigned int, Entity*>& xEntityMap = Entity::GetEntityList();
	std::map<const unsigned int, Entity*>::const_iterator xConstIter;

	// Loop over all entities in scene
	for (xConstIter = xEntityMap.begin(); xConstIter != xEntityMap.end(); xConstIter++)
	{
		Entity* pTarget = xConstIter->second;
		if (!pTarget)
		{
			return glm::vec3(0); // early out
		}
		if (pTarget->GetEntityID() != GetOwnerEntity()->GetEntityID())
		{
			TransformComponent* pTargetTransform = static_cast<TransformComponent*>(pTarget->FindComponentOfType(TRANSFORM));
			BrainComponent* pTargetBrain = static_cast<BrainComponent*>(pTarget->FindComponentOfType(BRAIN));

			if (!pTargetTransform)
			{
				return glm::vec3(0); // early out
			}
			// Find distance
			glm::vec3 v3TargetPos = pTargetTransform->GetEntityMatrixRow(POSITION_VECTOR);
			float fDistanceBetween = glm::length(v3TargetPos - v3LocalPos);

			// check the distance is within our neighbourhood
			if (fDistanceBetween < fNEIGHBOURHOOD_RADIUS)
			{
				// increment the values for the behaviours of the boids
				v3SeparationVel += (v3LocalPos - v3TargetPos);
				v3AllignmentVel += pTargetBrain->GetCurrentVelocity();
				v3CohesionVel += v3TargetPos;
				uNeighbourCount++;
			}
		}
	}

	//-------------------------Calculate Forces----------------------------\\

	glm::vec3 v3FinalForce(0.0f);

	// behaviour force calculations
	glm::vec3 v3WanderForce = CalculateWanderForce(v3Forward, v3LocalPos) * wanderWeight;
	glm::vec3 v3SeparationForce = CalculateSeparationForce(v3SeparationVel, uNeighbourCount) * separationWeight; // Add modifiers to the ends to determine how much of each happens
	glm::vec3 v3AllignmentForce = CalculateAlignmentForce(v3AllignmentVel, uNeighbourCount) * allignmentWeight;
	glm::vec3 v3CohesionForce = CalculateCohesionForce(v3LocalPos, v3CohesionVel, uNeighbourCount) * cohesionWeight;

	v3FinalForce = v3WanderForce + v3CohesionForce + v3AllignmentForce + v3SeparationForce;
	//----------------------------------------------------------------------\\

	return glm::vec3(v3FinalForce);
}

glm::vec3 BrainComponent::CalculateSeekForce(const glm::vec3& v3Target, const glm::vec3& v3CurrentPos) const
{
	// Calculate target direction
	glm::vec3 v3TargetDirection(v3Target - v3CurrentPos);
	if (glm::length(v3Target) > 0.0f)
	{
		v3TargetDirection = glm::normalize(v3TargetDirection);
	}

	// Calculate new vel
	glm::vec3 v3NewVel = v3TargetDirection * fSPEED;

	return (v3NewVel - m_v3CurrentVelocity);
}

glm::vec3 BrainComponent::CalculateFleeForce(const glm::vec3& v3Target, const glm::vec3& v3CurrentPos) const
{
	// Calculate target direction
	glm::vec3 v3TargetDirection(v3CurrentPos - v3Target);
	if (glm::length(v3Target) > 0.0f)
	{
		v3TargetDirection = glm::normalize(v3TargetDirection);
	}

	// Calculate new vel
	glm::vec3 v3NewVel = v3TargetDirection * fSPEED;

	return (v3NewVel - m_v3CurrentVelocity);
}

glm::vec3 BrainComponent::AvoidBox(const glm::vec3& v3Target, const glm::vec3& v3CurrentPos) const
{
	// Calculate target direction
	glm::vec3 v3TargetDirection(v3CurrentPos - v3Target);
	if (glm::length(v3Target) > 0.0f)
	{
		v3TargetDirection = glm::normalize(v3TargetDirection);
	}

	float distance = glm::distance(v3CurrentPos, v3Target);

	// if the boid is in a certain range of the box, give it a force to move away
	if (distance > boxSize && distance < boxRadius)
	{
		return (v3TargetDirection * separationWeight);
	}

	//if the boid isnt close to the box then dont add any additional force
	return (glm::vec3(0));
}

glm::vec3 BrainComponent::CalculateWanderForce(const glm::vec3& v3Forward, const glm::vec3& v3CurrentPos)
{
	// Project a point in front of it, for the centre of the sphere
	glm::vec3 v3SphereOrigin = v3CurrentPos + (v3Forward * fCIRCLE_FORWARD_MULTIPLIER);

	if (glm::length(m_v3WanderPoint) == 0.0f)
	{
		// Find random point on a sphere
		glm::vec3 v3RandomPointOnSphere = glm::sphericalRand(fWANDER_RADIUS);

		// Add the random point to the sphere origin
		m_v3WanderPoint = v3SphereOrigin + v3RandomPointOnSphere;
	}

	// Calculate
	glm::vec3 v3DirToTarget = glm::normalize(m_v3WanderPoint - v3SphereOrigin) * fWANDER_RADIUS;

	// Finding the final target point
	m_v3WanderPoint = v3SphereOrigin + v3DirToTarget;

	// Add jitter vector
	m_v3WanderPoint += glm::sphericalRand(fJITTER);

	return CalculateSeekForce(m_v3WanderPoint, v3CurrentPos);

}

/// <summary>
/// Calculate the force of separation from the other boids
/// </summary>
glm::vec3 BrainComponent::CalculateSeparationForce(glm::vec3 v3SeparationVel, unsigned int uNeighbourCount)
{

	if (glm::length(v3SeparationVel) > 0.0f)
	{
		v3SeparationVel /= uNeighbourCount;
		v3SeparationVel = glm::normalize(v3SeparationVel);
	}

	return v3SeparationVel;
}

/// <summary>
/// calcualte the force to keep all the bodis moving in the same direction
/// </summary>
glm::vec3 BrainComponent::CalculateAlignmentForce(glm::vec3 v3AllignmentVel, unsigned int uNeighbourCount)
{

	if (glm::length(v3AllignmentVel) > 0.0f)
	{
		v3AllignmentVel /= uNeighbourCount;
		v3AllignmentVel = glm::normalize(v3AllignmentVel);
	}

	return v3AllignmentVel;
}

/// <summary>
/// calculate the force to keep all the boids grouped together as much as they can be
/// </summary>
glm::vec3 BrainComponent::CalculateCohesionForce(glm::vec3 v3LocalPos, glm::vec3 v3CohesionVel, unsigned int uNeighbourCount)
{
	if (glm::length(v3CohesionVel) > 0.0f)
	{
		v3CohesionVel /= uNeighbourCount;
		v3CohesionVel = glm::normalize(v3CohesionVel - v3LocalPos);
	}

	return v3CohesionVel;
}

/// <summary>
/// calculates if the boids are near the edge of the bounding box and gives them a force to flee from the wall they are close to
/// </summary>
glm::vec3 BrainComponent::UpdateBoundsFleeForce(float a_fBoundsSize, glm::vec3 a_v3LocalPos)
{	
	glm::vec3 v3BoundsFleeForce(0.0f);

	float fleeForce = m_v3UpperVelClamp.x / 2;
	float fEarlySeparationDist = 0.5f;

	// check if the boid is near any of the walls
	bool bXBoundsPos = (a_v3LocalPos.x > a_fBoundsSize - fEarlySeparationDist);
	bool bXBoundsNeg = (a_v3LocalPos.x < -a_fBoundsSize + fEarlySeparationDist);
	bool bYBoundsPos = (a_v3LocalPos.y > a_fBoundsSize - fEarlySeparationDist);
	bool bYBoundsNeg = (a_v3LocalPos.y < -a_fBoundsSize + fEarlySeparationDist);
	bool bZBoundsPos = (a_v3LocalPos.z > a_fBoundsSize - fEarlySeparationDist);
	bool bZBoundsNeg = (a_v3LocalPos.z < -a_fBoundsSize + fEarlySeparationDist);

	// Depending on which wall the boid is near, give it a force to go in the opposite direction
	if (bXBoundsPos)
	{
		v3BoundsFleeForce.x = -fleeForce;
	}
	else if (bXBoundsNeg)
	{
		v3BoundsFleeForce.x = fleeForce;
	}
	else if (bYBoundsPos)
	{
		v3BoundsFleeForce.y = -fleeForce;
	}
	else if (bYBoundsNeg)
	{
		v3BoundsFleeForce.y = fleeForce;
	}
	else if (bZBoundsPos)
	{
		v3BoundsFleeForce.z = -fleeForce;
	}
	else if (bZBoundsNeg)
	{
		v3BoundsFleeForce.z = fleeForce;
	}

	// if the boid is near two walls, it gives the force in the direction of which wall its closer too
	if (bXBoundsPos && bYBoundsPos)
	{
		if (a_v3LocalPos.x > a_v3LocalPos.y) v3BoundsFleeForce.y = 0.0f;
		else v3BoundsFleeForce.x = 0.0f;
	}
	else if (bXBoundsPos && bZBoundsPos)
	{
		if (a_v3LocalPos.x > a_v3LocalPos.z) v3BoundsFleeForce.z = 0.0f;
		else v3BoundsFleeForce.x = 0.0f;
	}
	else if (bYBoundsPos && bZBoundsPos)
	{
		if (a_v3LocalPos.y > a_v3LocalPos.z) v3BoundsFleeForce.z = 0.0f;
		else v3BoundsFleeForce.y = 0.0f;
	}

	else if (bXBoundsNeg && bYBoundsNeg)
	{
		if (a_v3LocalPos.x < a_v3LocalPos.y) v3BoundsFleeForce.y = 0.0f;
		else v3BoundsFleeForce.x = 0.0f;
	}
	else if (bXBoundsNeg && bZBoundsNeg)
	{
		if (a_v3LocalPos.x < a_v3LocalPos.z) v3BoundsFleeForce.z = 0.0f;
		else v3BoundsFleeForce.x = 0.0f;
	}
	else if (bYBoundsNeg && bZBoundsNeg)
	{
		if (a_v3LocalPos.y < a_v3LocalPos.z) v3BoundsFleeForce.z = 0.0f;
		else v3BoundsFleeForce.y = 0.0f;
	}

	return v3BoundsFleeForce;
}
