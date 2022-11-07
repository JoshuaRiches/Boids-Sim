// This files header
#include "Component.h"

// Project Includes
#include "Entity.h"

// constructor
Component::Component(Entity* a_pOwner) : m_pOwnerEntity(a_pOwner), m_eComponentType(NONE)
{

}