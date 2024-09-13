#include "Mole.h"
#include <Engine/Physics/sRigidBodyState.h>
#include <Engine/Asserts/Asserts.h>
#include <Engine/Logging/Logging.h>

bool eae6320::Mole::GetActive()
{
	return m_active;
}

void eae6320::Mole::SetActive(bool i_active)
{
	if (i_active) m_renderableObject->SetEffectIndex(1);
	else m_renderableObject->SetEffectIndex(0);
	m_active = i_active;
}

eae6320::GameObjects::cRenderableObject* eae6320::Mole::GetRenderableObject()
{
	return m_renderableObject;
}

void eae6320::Mole::Initialize()
{
	Physics::sRigidBodyState gameObjectRigidBodyState;
	{
		gameObjectRigidBodyState.position = Math::sVector(0.0f, 0.0f, 0.0f);
		gameObjectRigidBodyState.orientation = Math::cQuaternion();
		gameObjectRigidBodyState.velocity = Math::sVector(0.0f, 0.0f, 0.0f);
		gameObjectRigidBodyState.acceleration = Math::sVector(0.0f, 0.0f, 0.0f);
	}
	eae6320::Graphics::cEffect* activeEffect;
	{
		eae6320::Graphics::cShader* vertexShader;
		eae6320::Graphics::cShader* fragmentShader;
		if (!eae6320::Graphics::cShader::Load("data/Shaders/Vertex/standard.shader",
			vertexShader, eae6320::Graphics::eShaderType::Vertex))
		{
			EAE6320_ASSERTF(false, "Can't initialize shading data without vertex shader");
		}
		if (!eae6320::Graphics::cShader::Load("data/Shaders/Fragment/MoleActive.shader",
			fragmentShader, eae6320::Graphics::eShaderType::Fragment))
		{
			EAE6320_ASSERTF(false, "Can't initialize shading data without fragment shader");
		}
		eae6320::Graphics::cEffect::Load(activeEffect, vertexShader, fragmentShader);
	}
	eae6320::Graphics::cEffect* inactiveEffect;
	{
		eae6320::Graphics::cShader* vertexShader;
		eae6320::Graphics::cShader* fragmentShader;
		if (!eae6320::Graphics::cShader::Load("data/Shaders/Vertex/standard.shader",
			vertexShader, eae6320::Graphics::eShaderType::Vertex))
		{
			EAE6320_ASSERTF(false, "Can't initialize shading data without vertex shader");
		}
		if (!eae6320::Graphics::cShader::Load("data/Shaders/Fragment/MoleInactive.shader",
			fragmentShader, eae6320::Graphics::eShaderType::Fragment))
		{
			EAE6320_ASSERTF(false, "Can't initialize shading data without fragment shader");
		}
		eae6320::Graphics::cEffect::Load(inactiveEffect, vertexShader, fragmentShader);
	}
	eae6320::Graphics::cMesh* mesh;
	{
		eae6320::Graphics::cMesh::Load(mesh, "data/Meshes/Mole.mesh");
	}
	Graphics::cEffect** ppEffect = new Graphics::cEffect*[2];
	ppEffect[0] = inactiveEffect;
	ppEffect[1] = activeEffect;
	Graphics::cMesh** ppMesh = new Graphics::cMesh*[1];
	ppMesh[0] = mesh;
	if (!eae6320::GameObjects::cRenderableObject::Load(m_renderableObject, gameObjectRigidBodyState, ppEffect, 2, ppMesh, 1))
	{
		EAE6320_ASSERTF(false, "Can't initialize renderableObject");
	}
	m_initialized = true;
}

void eae6320::Mole::CleanUp()
{
	if (m_renderableObject != nullptr)
	{
		m_renderableObject->DecrementReferenceCount();
		m_renderableObject = nullptr;
	}
}

void eae6320::Mole::SetPosition(Math::sVector i_position)
{
	m_renderableObject->SetPosition(i_position);
}