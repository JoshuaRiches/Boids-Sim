#ifndef ENTITY_H
#define ENTITY_H

// Project Includes
#include "Component.h"

// std includes
#include <vector>
#include <map>

// Forward declerations
class Shader;

class Entity
{
public:
	Entity();

	// functions for doing processes each frame and rendering
	virtual void Update(float a_fDeltaTime, float a_fBoundingBoxSize);
	virtual void Draw(Shader* a_pShader);

	// adds a component to the entity
	void AddComponent(Component* a_pComponent);
	// returns the instance of the specified component
	Component* FindComponentOfType(COMPONENT_TYPE a_eComponentType) const;

	// stores all the entities in scene
	static const std::map<const unsigned int, Entity*>& GetEntityList() { return s_xEntityList; }
	// removed specified entity from the map of entities
	static void RemoveEntity(std::map<const unsigned int, Entity*>::const_iterator xIter) { s_xEntityList.erase(xIter); s_uEntityCount--; }

	// gets id of the entity
	unsigned int GetEntityID() { return m_uEntityID; }

private:
	unsigned int m_uEntityID;
	std::vector<Component*> m_apComponentList;

	static unsigned int s_uEntityCount;
	static std::map<const unsigned int, Entity*> s_xEntityList;
};

#endif //!ENTITY_H