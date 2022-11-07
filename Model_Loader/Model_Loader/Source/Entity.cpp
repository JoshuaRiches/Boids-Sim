#include "Entity.h"

// typedefs
typedef std::pair<const unsigned int, Entity*> EntityPair;

// Statics
unsigned int Entity::s_uEntityCount = 0;
std::map<const unsigned int, Entity*> Entity::s_xEntityList;

Entity::Entity()
{
	// Increment Entity ID
	m_uEntityID = s_uEntityCount++;

	// Add this entity to the list
	s_xEntityList.insert(EntityPair(m_uEntityID, this));
}

/// <summary>
/// function called each frame
/// </summary>
/// <param name="a_fDeltaTime"> time between frames </param>
/// <param name="a_fBoundingBoxSize"> size of the bounding box </param>
void Entity::Update(float a_fDeltaTime, float a_fBoundingBoxSize)
{
	std::vector<Component*>::iterator xIter;
	// calls the update function on each of the components on the entity
	for (xIter = m_apComponentList.begin(); xIter < m_apComponentList.end(); xIter++)
	{
		Component* pComponent = *xIter;
		if (pComponent)
		{
			pComponent->Update(a_fDeltaTime, a_fBoundingBoxSize);
		}
	}
}

/// <summary>
/// calls the render functions
/// </summary>
void Entity::Draw(Shader* a_pShader)
{
	std::vector<Component*>::iterator xIter;
	// calls the draw function on all the components on the entity
	for (xIter = m_apComponentList.begin(); xIter < m_apComponentList.end(); xIter++)
	{
		Component* pComponent = *xIter;
		if (pComponent)
		{
			pComponent->Draw(a_pShader);
		}
	}
}

/// <summary>
/// Adds a component to the list of components for this entity
/// </summary>
void Entity::AddComponent(Component* a_pComponent) 
{ 
	m_apComponentList.push_back(a_pComponent); 
}


/// <summary>
/// searches through the components on the entity looking for the specified one 
/// </summary>
/// <param name="a_eComponentType"> component to find </param>
Component* Entity::FindComponentOfType(COMPONENT_TYPE a_eComponentType) const
{
	std::vector<Component*>::const_iterator xIter;
	for (xIter = m_apComponentList.begin(); xIter < m_apComponentList.end(); xIter++)
	{
		Component* pComponent = *xIter;
		if (pComponent && pComponent->GetComponentType() == a_eComponentType)
		{
			return pComponent;
		}
	}

	return nullptr;
}