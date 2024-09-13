// Includes
//=========

#include "cRenderTarget.h"

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
	class cRenderTargetTracker
	{
		// Interface
		//----------

	public:

		// Access
		void AddRenderTarget(const eae6320::Graphics::cRenderTarget* const i_renderTarget);
		void RemoveRenderTarget(const eae6320::Graphics::cRenderTarget* const i_renderTarget);
		// Initializate / Clean Up
		cRenderTargetTracker() = default;
		~cRenderTargetTracker();

		// Data
		//-----

	private:

		std::vector<const eae6320::Graphics::cRenderTarget*> m_renderTargets;
		eae6320::Concurrency::cMutex m_mutex;

		// Implementation
		//---------------

	private:

		// Initializate / Clean Up
		cRenderTargetTracker(const cRenderTargetTracker&) = delete;
		cRenderTargetTracker(cRenderTargetTracker&&) = delete;
		cRenderTargetTracker& operator =(const cRenderTargetTracker&) = delete;
		cRenderTargetTracker& operator =(cRenderTargetTracker&&) = delete;
	} s_renderTargetTracker;
}


eae6320::cResult eae6320::Graphics::cRenderTarget::Load(cRenderTarget*& o_renderTarget, const sInitializationParameters& i_initializationParameters)
{
	auto result = Results::Success;

	cRenderTarget* newRenderTarget = nullptr;
	cScopeGuard scopeGuard([&o_renderTarget, &result, &newRenderTarget]
		{
			if (result)
			{
				EAE6320_ASSERT(newRenderTarget != nullptr);
				o_renderTarget = newRenderTarget;
			}
			else
			{
				if (newRenderTarget)
				{
					newRenderTarget->DecrementReferenceCount();
					newRenderTarget = nullptr;
				}
				o_renderTarget = nullptr;
			}
		});

	// Allocate a new renderTarget
	{
		newRenderTarget = new (std::nothrow) cRenderTarget();
		if (!newRenderTarget)
		{
			result = Results::OutOfMemory;
			EAE6320_ASSERTF(false, "Couldn't allocate memory for the renderTarget");
			Logging::OutputError("Failed to allocate memory for the renderTarget");
			return result;
		}
	}

	// Initialize the platform-specific graphics API renderTarget object
	if (!(result = newRenderTarget->Initialize(i_initializationParameters)))
	{
		EAE6320_ASSERTF(false, "Initialization of new renderTarget failed");
		return result;
	}

	return result;
}

// Implementation
//===============

// Initialize / Clean Up
//----------------------

eae6320::Graphics::cRenderTarget::cRenderTarget()
{
	s_renderTargetTracker.AddRenderTarget(this);
}

eae6320::Graphics::cRenderTarget::~cRenderTarget()
{
	EAE6320_ASSERT(m_referenceCount == 0);
	s_renderTargetTracker.RemoveRenderTarget(this);
	const auto result = CleanUp();
	EAE6320_ASSERT(result);
}

// Helper Class Definition
//========================

namespace
{
	// Interface
	//----------

	// Access

	void cRenderTargetTracker::AddRenderTarget(const eae6320::Graphics::cRenderTarget* const i_renderTarget)
	{
		EAE6320_ASSERT(i_renderTarget != nullptr);
		eae6320::Concurrency::cMutex::cScopeLock scopeLock(m_mutex);
#ifdef EAE6320_ASSERTS_AREENABLED
		for (const auto* const renderTarget : m_renderTargets)
		{
			EAE6320_ASSERTF(renderTarget != i_renderTarget, "This renderTarget is already being tracked");
		}
#endif
		m_renderTargets.push_back(i_renderTarget);
	}

	void cRenderTargetTracker::RemoveRenderTarget(const eae6320::Graphics::cRenderTarget* const i_renderTarget)
	{
		eae6320::Concurrency::cMutex::cScopeLock scopeLock(m_mutex);
		const auto renderTargetCount = m_renderTargets.size();
		for (std::remove_const<decltype(renderTargetCount)>::type i = 0; i < renderTargetCount; ++i)
		{
			if (m_renderTargets[i] == i_renderTarget)
			{
				m_renderTargets.erase(m_renderTargets.begin() + i);
				return;
			}
		}
		EAE6320_ASSERTF(false, "This renderTarget wasn't being tracked");
	}

	// Initialize / Clean Up

	cRenderTargetTracker::~cRenderTargetTracker()
	{
		eae6320::Concurrency::cMutex::cScopeLock scopeLock(m_mutex);
		const auto haveAnyRenderTargetsNotBeenDeleted = !m_renderTargets.empty();
		if (haveAnyRenderTargetsNotBeenDeleted)
		{
			EAE6320_ASSERTF(!haveAnyRenderTargetsNotBeenDeleted,
				"If you are seeing this message there are %i renderTargets that haven't been deleted. You probably have a reference counting bug!", m_renderTargets.size());
			eae6320::Logging::OutputError("Not every renderTarget was released. The program will now crash!");
			constexpr int* const invalidAddress = 0x0;
			*invalidAddress = 123;	// Intentional crash! You have a bug that must be fixed!
		}
	}
}
