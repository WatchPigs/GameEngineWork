// Includes
//=========

#include "cCamera.h"

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
	class cCameraTracker
	{
		// Interface
		//----------

	public:

		// Access
		void AddCamera(const eae6320::GameObjects::cCamera* const i_camera);
		void RemoveCamera(const eae6320::GameObjects::cCamera* const i_camera);
		// Initializate / Clean Up
		cCameraTracker() = default;
		~cCameraTracker();

		// Data
		//-----

	private:

		std::vector<const eae6320::GameObjects::cCamera*> m_cameras;
		eae6320::Concurrency::cMutex m_mutex;

		// Implementation
		//---------------

	private:

		// Initializate / Clean Up
		cCameraTracker(const cCameraTracker&) = delete;
		cCameraTracker(cCameraTracker&&) = delete;
		cCameraTracker& operator =(const cCameraTracker&) = delete;
		cCameraTracker& operator =(cCameraTracker&&) = delete;
	} s_cameraTracker;
}

eae6320::cResult eae6320::GameObjects::cCamera::Load(cCamera*& o_camera, const Physics::sRigidBodyState i_rigidBodyState, Math::cMatrix_transformation i_transform_cameraToProjected)
{
	auto result = Results::Success;

	cCamera* newCamera = nullptr;
	cScopeGuard scopeGuard([&o_camera, &result, &newCamera]
		{
			if (result)
			{
				EAE6320_ASSERT(newCamera != nullptr);
				o_camera = newCamera;
			}
			else
			{
				if (newCamera)
				{
					newCamera->DecrementReferenceCount();
					newCamera = nullptr;
				}
				o_camera = nullptr;
			}
		});

	// Allocate a new camera
	{
		newCamera = new (std::nothrow) cCamera();
		if (!newCamera)
		{
			result = Results::OutOfMemory;
			EAE6320_ASSERTF(false, "Couldn't allocate memory for the camera");
			Logging::OutputError("Failed to allocate memory for the camera");
			return result;
		}
	}

	// Initialize the platform-specific graphics API camera object
	if (!(result = newCamera->Initialize(i_rigidBodyState, i_transform_cameraToProjected)))
	{
		EAE6320_ASSERTF(false, "Initialization of new camera failed");
		return result;
	}

	return result;
}

// Implementation
//===============

// Initialize / Clean Up
//----------------------

eae6320::GameObjects::cCamera::cCamera()
{
	s_cameraTracker.AddCamera(this);
}

eae6320::GameObjects::cCamera::~cCamera()
{
	EAE6320_ASSERT(m_referenceCount == 0);
	s_cameraTracker.RemoveCamera(this);
	const auto result = CleanUp();
	EAE6320_ASSERT(result);
}

eae6320::cResult eae6320::GameObjects::cCamera::Initialize(const Physics::sRigidBodyState i_rigidBodyState, const Math::cMatrix_transformation i_transform_cameraToProjected)
{
	m_rigidBodyState = i_rigidBodyState;
	m_transform_worldToCamera = Math::cMatrix_transformation::CreateWorldToCameraTransform(m_rigidBodyState.orientation, m_rigidBodyState.position);
	m_transform_cameraToProjected = i_transform_cameraToProjected;

	return Results::Success;
}

eae6320::cResult eae6320::GameObjects::cCamera::CleanUp()
{
	return Results::Success;
}


// Update Transform
//----------------------

void eae6320::GameObjects::cCamera::UpdateSimulation(const float i_secondCountToIntegrate)
{
	m_rigidBodyState.Update(i_secondCountToIntegrate);
	m_transform_worldToCamera = Math::cMatrix_transformation::CreateWorldToCameraTransform(m_rigidBodyState.orientation, m_rigidBodyState.position);
}

// Predict Transform for render
//----------------------
void eae6320::GameObjects::cCamera::PredictForRender(const float i_secondCountToExtrapolate)
{
	m_transform_worldToCamera = Math::cMatrix_transformation::CreateWorldToCameraTransform(m_rigidBodyState.PredictFutureOrientation(i_secondCountToExtrapolate), m_rigidBodyState.PredictFuturePosition(i_secondCountToExtrapolate));
}

// Input Velocity
//----------------------
void eae6320::GameObjects::cCamera::SetVelocity(Math::sVector i_velocity)
{
	m_rigidBodyState.velocity = i_velocity;
}

// Get Data for render
//----------------------
eae6320::Math::cMatrix_transformation eae6320::GameObjects::cCamera::GetTransformWorldToCamera()
{
	return m_transform_worldToCamera;
}

eae6320::Math::cMatrix_transformation eae6320::GameObjects::cCamera::GetTransformCameraToProjected()
{
	return m_transform_cameraToProjected;
}

// Helper Class Definition
//========================

namespace
{
	// Interface
	//----------

	// Access

	void cCameraTracker::AddCamera(const eae6320::GameObjects::cCamera* const i_camera)
	{
		EAE6320_ASSERT(i_camera != nullptr);
		eae6320::Concurrency::cMutex::cScopeLock scopeLock(m_mutex);
#ifdef EAE6320_ASSERTS_AREENABLED
		for (const auto* const camera : m_cameras)
		{
			EAE6320_ASSERTF(camera != i_camera, "This camera is already being tracked");
		}
#endif
		m_cameras.push_back(i_camera);
	}

	void cCameraTracker::RemoveCamera(const eae6320::GameObjects::cCamera* const i_camera)
	{
		eae6320::Concurrency::cMutex::cScopeLock scopeLock(m_mutex);
		const auto cameraCount = m_cameras.size();
		for (std::remove_const<decltype(cameraCount)>::type i = 0; i < cameraCount; ++i)
		{
			if (m_cameras[i] == i_camera)
			{
				m_cameras.erase(m_cameras.begin() + i);
				return;
			}
		}
		EAE6320_ASSERTF(false, "This camera wasn't being tracked");
	}

	// Initialize / Clean Up

	cCameraTracker::~cCameraTracker()
	{
		eae6320::Concurrency::cMutex::cScopeLock scopeLock(m_mutex);
		const auto haveAnyCamerasNotBeenDeleted = !m_cameras.empty();
		if (haveAnyCamerasNotBeenDeleted)
		{
			EAE6320_ASSERTF(!haveAnyCamerasNotBeenDeleted,
				"If you are seeing this message there are %i cameras that haven't been deleted. You probably have a reference counting bug!", m_cameras.size());
			eae6320::Logging::OutputError("Not every camera was released. The program will now crash!");
			constexpr int* const invalidAddress = 0x0;
			*invalidAddress = 123;	// Intentional crash! You have a bug that must be fixed!
		}
	}
}
