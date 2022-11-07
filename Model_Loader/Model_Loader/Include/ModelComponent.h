#ifndef MODELCOMPONENT_H
#define MODELCOMPONENT_H

#include "Component.h"

class Model;

/// <summary>
/// Header file for the model component
/// </summary>
class ModelComponent :
    public Component
{
public:
    // constructor and destructor
    ModelComponent(Entity* a_pOwner);
    ~ModelComponent();

    // functions inherited from the component parent
    virtual void Update(float a_fDeltaTime, float a_fBoundingBox) {};
    virtual void Draw(Shader* a_pShader);

    // set the model file to be used for the boids
    void SetModel(Model* pNewModel) { m_pModelData = pNewModel; }
    // set the scale of the model
    void SetScale(float a_fNewScale) { m_fModelScale = a_fNewScale; }

private:
    Model* m_pModelData;
    float m_fModelScale;
};

#endif // !MODELCOMPONENT_H