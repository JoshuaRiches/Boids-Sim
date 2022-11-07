#include "ModelComponent.h"

// LearnOpenGL includes
#include <learnopengl/shader.h>
#include <learnopengl/model.h>
// GLM includes
#include <glm/gtc/matrix_transform.hpp>
// Project Includes
#include "TransformComponent.h"
#include "Entity.h"

// TypeDef
typedef Component PARENT;

ModelComponent::ModelComponent(Entity* a_pOwner) : PARENT(a_pOwner), m_pModelData(nullptr), m_fModelScale(0.0f)
{

}

ModelComponent::~ModelComponent()
{

}

void ModelComponent::Draw(Shader* a_pShader) 
{
    // NUll check
    if (!a_pShader)
    {
        return; // Early out
    }

    if (!m_pModelData)
    {
        return; // Early Out
    }

    // get Transform component
    TransformComponent* pTransformComponent = static_cast<TransformComponent*>(m_pOwnerEntity->FindComponentOfType(TRANSFORM));
    if (!pTransformComponent)
    {
        return; // Early Out
    }

    // render the loaded model
    glm::mat4 m4ModelMatrix = pTransformComponent->GetEntityMatrix();
    m4ModelMatrix = glm::scale(m4ModelMatrix, glm::vec3(m_fModelScale, m_fModelScale, m_fModelScale));
    a_pShader->setMat4("model", m4ModelMatrix);
    m_pModelData->Draw(*a_pShader);
}