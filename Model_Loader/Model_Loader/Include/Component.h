#ifndef COMPONENT_H
#define COMPONENT_H

// Forward decleration
class Entity;
class Shader;

enum COMPONENT_TYPE
{
	NONE,
	TRANSFORM,
	MODEL,
	BRAIN,
};

class Component
{
public:
	Component(Entity* a_pOwner);

	// functions for doing proceses each frame and rendering
	virtual void Update(float a_fDeltaTime, float a_fBoundingBoxSize) = 0;
	virtual void Draw(Shader* pShader) = 0;

	// returns owner entity
	inline Entity* GetOwnerEntity() { return m_pOwnerEntity; }
	// returns the type of the component
	inline COMPONENT_TYPE GetComponentType() { return m_eComponentType; }

protected:
	Entity* m_pOwnerEntity;
	COMPONENT_TYPE m_eComponentType;
};

#endif // !COMPONENT_H