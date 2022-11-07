#ifndef TRANSFORMCOMPONENT_H
#define TRANSFORMCOMPONENT_H

// Includes
#include "Component.h"
#include <glm/ext.hpp>

enum MATRIX_ROW
{
	RIGHT_VECTOR,
	UP_VECTOR,
	FORWARD_VECTOR,
	POSITION_VECTOR
};

class TransformComponent : public Component
{
public:
	// constructor and destructor
	TransformComponent(Entity* a_pOwner);
	~TransformComponent();

	// functions inherited from component
	virtual void Update(float a_fDeltaTime,  float a_fBoundingBoxSize) {};
	virtual void Draw(Shader* pShader) {};

	// returns the entity matrix
	const glm::mat4& GetEntityMatrix() { return m_m4EntityMatrix; }

	// sets the value of the specified matrix row
	void SetEntityMatrixRow(MATRIX_ROW a_eRow, glm::vec3 a_v3Vec);
	// returns a row of the matrix
	glm::vec3 GetEntityMatrixRow(MATRIX_ROW a_eRow);

private:
	glm::mat4 m_m4EntityMatrix;

};


#endif // !TRANSFORMCOMPONENT_H