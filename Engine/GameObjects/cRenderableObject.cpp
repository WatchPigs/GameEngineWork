// Includes
//=========

#include "cRenderableObject.h"

#include <Engine/Asserts/Asserts.h>
#include <Engine/Concurrency/cMutex.h>
#include <Engine/Logging/Logging.h>
#include <Engine/ScopeGuard/cScopeGuard.h>
#include <new>
#include <vector>

namespace
{
	// This exists for our class to help you catch reference counting problems.
	// In a real game there would be more sophisticated methods that would work for all asset types.
	class cRenderableObjectTracker
	{
		// Interface
		//----------

	public:

		// Access
		void AddRenderableObject(const eae6320::GameObjects::cRenderableObject* const i_renderableObject);
		void RemoveRenderableObject(const eae6320::GameObjects::cRenderableObject* const i_renderableObject);
		// Initializate / Clean Up
		cRenderableObjectTracker() = default;
		~cRenderableObjectTracker();

		// Data
		//-----

	private:

		std::vector<const eae6320::GameObjects::cRenderableObject*> m_renderableObjects;
		eae6320::Concurrency::cMutex m_mutex;

		// Implementation
		//---------------

	private:

		// Initializate / Clean Up
		cRenderableObjectTracker(const cRenderableObjectTracker&) = delete;
		cRenderableObjectTracker(cRenderableObjectTracker&&) = delete;
		cRenderableObjectTracker& operator =(const cRenderableObjectTracker&) = delete;
		cRenderableObjectTracker& operator =(cRenderableObjectTracker&&) = delete;
	} s_renderableObjectTracker;
}

eae6320::cResult eae6320::GameObjects::cRenderableObject::Load(cRenderableObject*& o_renderableObject, const Physics::sRigidBodyState i_rigidBodyState, Graphics::cEffect** i_ppEffect, unsigned int i_effctsCount, Graphics::cMesh** i_ppMesh, unsigned int i_meshesCount)
{
	auto result = Results::Success;

	cRenderableObject* newRenderableObject = nullptr;
	cScopeGuard scopeGuard([&o_renderableObject, &result, &newRenderableObject]
		{
			if (result)
			{
				EAE6320_ASSERT(newRenderableObject != nullptr);
				o_renderableObject = newRenderableObject;
			}
			else
			{
				if (newRenderableObject)
				{
					newRenderableObject->DecrementReferenceCount();
					newRenderableObject = nullptr;
				}
				o_renderableObject = nullptr;
			}
		});

	// Allocate a new renderableObject
	{
		newRenderableObject = new (std::nothrow) cRenderableObject();
		if (!newRenderableObject)
		{
			result = Results::OutOfMemory;
			EAE6320_ASSERTF(false, "Couldn't allocate memory for the renderableObject");
			Logging::OutputError("Failed to allocate memory for the renderableObject");
			return result;
		}
	}

	// Initialize the platform-specific graphics API renderableObject object
	if (!(result = newRenderableObject->Initialize(i_rigidBodyState, i_ppEffect, i_effctsCount, i_ppMesh, i_meshesCount)))
	{
		EAE6320_ASSERTF(false, "Initialization of new renderableObject failed");
		return result;
	}

	return result;
}

// Implementation
//===============

// Initialize / Clean Up
//----------------------

eae6320::GameObjects::cRenderableObject::cRenderableObject()
{
	s_renderableObjectTracker.AddRenderableObject(this);
}

eae6320::GameObjects::cRenderableObject::~cRenderableObject()
{
	EAE6320_ASSERT(m_referenceCount == 0);
	s_renderableObjectTracker.RemoveRenderableObject(this);
	const auto result = CleanUp();
	EAE6320_ASSERT(result);
}

eae6320::cResult eae6320::GameObjects::cRenderableObject::Initialize(const Physics::sRigidBodyState i_rigidBodyState, Graphics::cEffect** i_ppEffect, unsigned int i_effctsCount, Graphics::cMesh** i_ppMesh, unsigned int i_meshesCount)
{
	m_rigidBodyState = i_rigidBodyState;
	m_ppEffect = i_ppEffect;
	m_ppMesh = i_ppMesh;
	m_effctsCount = i_effctsCount;
	m_meshesCount = i_meshesCount;

	return Results::Success;
}

eae6320::cResult eae6320::GameObjects::cRenderableObject::CleanUp()
{
	for (unsigned int i = 0; i < m_effctsCount; i++)
	{
		if (m_ppEffect[i] != nullptr)
		{
			m_ppEffect[i]->DecrementReferenceCount();
			m_ppEffect[i] = nullptr;
		}
	}

	for (unsigned int i = 0; i < m_meshesCount; i++)
	{
		if (m_ppMesh[i] != nullptr)
		{
			m_ppMesh[i]->DecrementReferenceCount();
			m_ppMesh[i] = nullptr;
		}
	}

	delete m_ppEffect;
	delete m_ppMesh;

	return Results::Success;
}

// Update Transform
//----------------------

void eae6320::GameObjects::cRenderableObject::UpdateSimulation(const float i_secondCountToIntegrate)
{
	m_rigidBodyState.Update(i_secondCountToIntegrate);
}

// Predict Transform for render
//----------------------
eae6320::Math::cMatrix_transformation eae6320::GameObjects::cRenderableObject::PredictFutureTransform(const float i_secondCountToExtrapolate)
{
	return m_rigidBodyState.PredictFutureTransform(i_secondCountToExtrapolate);
}

// Input Velocity
//----------------------
void eae6320::GameObjects::cRenderableObject::SetVelocity(Math::sVector i_velocity)
{
	m_rigidBodyState.velocity = i_velocity;
}

// Input Position
//----------------------
void eae6320::GameObjects::cRenderableObject::SetPosition(Math::sVector i_position)
{
	m_rigidBodyState.position = i_position;
}

// Input set effect/mesh
//----------------------
void eae6320::GameObjects::cRenderableObject::SetEffectIndex(const unsigned int i_effectIndex)
{
	if ((i_effectIndex < m_effctsCount) && (m_effectIndex != i_effectIndex)) m_effectIndex = i_effectIndex;
}

void eae6320::GameObjects::cRenderableObject::SetMeshIndex(const unsigned int i_meshIndex)
{
	if ((i_meshIndex < m_meshesCount) && (m_meshIndex != i_meshIndex)) m_meshIndex = i_meshIndex;
}


// Get Data for render
//----------------------

eae6320::Graphics::sEffectDrawCallAndMesh eae6320::GameObjects::cRenderableObject::GetRenderData(const float i_elapsedSecondCount_sinceLastSimulationUpdate)
{
	Graphics::sEffectDrawCallAndMesh effectDrawCallAndMesh;
	effectDrawCallAndMesh.m_effect = m_ppEffect[m_effectIndex];
	effectDrawCallAndMesh.m_effect->IncrementReferenceCount();
	effectDrawCallAndMesh.m_constantData_drawCall.g_transform_localToWorld = PredictFutureTransform(i_elapsedSecondCount_sinceLastSimulationUpdate);
	effectDrawCallAndMesh.m_mesh = m_ppMesh[m_meshIndex];
	effectDrawCallAndMesh.m_mesh->IncrementReferenceCount();
	return effectDrawCallAndMesh;
}

// Helper Class Definition
//========================

namespace
{
	// Interface
	//----------

	// Access

	void cRenderableObjectTracker::AddRenderableObject(const eae6320::GameObjects::cRenderableObject* const i_renderableObject)
	{
		EAE6320_ASSERT(i_renderableObject != nullptr);
		eae6320::Concurrency::cMutex::cScopeLock scopeLock(m_mutex);
#ifdef EAE6320_ASSERTS_AREENABLED
		for (const auto* const renderableObject : m_renderableObjects)
		{
			EAE6320_ASSERTF(renderableObject != i_renderableObject, "This renderableObject is already being tracked");
		}
#endif
		m_renderableObjects.push_back(i_renderableObject);
	}

	void cRenderableObjectTracker::RemoveRenderableObject(const eae6320::GameObjects::cRenderableObject* const i_renderableObject)
	{
		eae6320::Concurrency::cMutex::cScopeLock scopeLock(m_mutex);
		const auto renderableObjectCount = m_renderableObjects.size();
		for (std::remove_const<decltype(renderableObjectCount)>::type i = 0; i < renderableObjectCount; ++i)
		{
			if (m_renderableObjects[i] == i_renderableObject)
			{
				m_renderableObjects.erase(m_renderableObjects.begin() + i);
				return;
			}
		}
		EAE6320_ASSERTF(false, "This renderableObject wasn't being tracked");
	}

	// Initialize / Clean Up

	cRenderableObjectTracker::~cRenderableObjectTracker()
	{
		eae6320::Concurrency::cMutex::cScopeLock scopeLock(m_mutex);
		const auto haveAnyRenderableObjectsNotBeenDeleted = !m_renderableObjects.empty();
		if (haveAnyRenderableObjectsNotBeenDeleted)
		{
			EAE6320_ASSERTF(!haveAnyRenderableObjectsNotBeenDeleted,
				"If you are seeing this message there are %i renderableObjects that haven't been deleted. You probably have a reference counting bug!", m_renderableObjects.size());
			eae6320::Logging::OutputError("Not every renderableObject was released. The program will now crash!");
			constexpr int* const invalidAddress = 0x0;
			*invalidAddress = 123;	// Intentional crash! You have a bug that must be fixed!
		}
	}
}