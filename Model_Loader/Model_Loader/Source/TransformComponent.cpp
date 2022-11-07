// This files header
#include "TransformComponent.h"

// Typedefs
typedef Component PARENT;

/// <summary>
/// Constructor to create an instance of a transform component
/// </summary>
TransformComponent::TransformComponent(Entity* a_pOwner) : PARENT(a_pOwner), m_m4EntityMatrix(glm::mat4(1.0f))
{
	m_eComponentType = TRANSFORM;
}

// destructor
TransformComponent::~TransformComponent()
{

}

/// <summary>
/// sets the value of the specified row in the entity matrix
/// </summary>
void TransformComponent::SetEntityMatrixRow(MATRIX_ROW a_eRow, glm::vec3 a_v3Vec)
{
	m_m4EntityMatrix[a_eRow] = glm::vec4(a_v3Vec, (a_eRow == POSITION_VECTOR ? 1.0f : 0.0f));
}

/// <summary>
/// returns the specified row in the entity matrix
/// </summary>
glm::vec3 TransformComponent::GetEntityMatrixRow(MATRIX_ROW a_eRow)
{
	glm::vec4 vBlue(0, 0, 1, 1);


	return m_m4EntityMatrix[a_eRow];
}