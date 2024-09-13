// Includes
//=========

#include "cEffect.h"

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
	class cEffectTracker
	{
		// Interface
		//----------

	public:

		// Access
		void AddEffect(const eae6320::Graphics::cEffect* const i_effect);
		void RemoveEffect(const eae6320::Graphics::cEffect* const i_effect);
		// Initializate / Clean Up
		cEffectTracker() = default;
		~cEffectTracker();

		// Data
		//-----

	private:

		std::vector<const eae6320::Graphics::cEffect*> m_effects;
		eae6320::Concurrency::cMutex m_mutex;

		// Implementation
		//---------------

	private:

		// Initializate / Clean Up
		cEffectTracker(const cEffectTracker&) = delete;
		cEffectTracker(cEffectTracker&&) = delete;
		cEffectTracker& operator =(const cEffectTracker&) = delete;
		cEffectTracker& operator =(cEffectTracker&&) = delete;
	} s_effectTracker;
}

eae6320::cResult eae6320::Graphics::cEffect::Load(cEffect*& o_effect, cShader* i_vertexShader, cShader* i_fragmentShader)
{
	auto result = Results::Success;

	cEffect* newEffect = nullptr;
	cScopeGuard scopeGuard([&o_effect, &result, &newEffect]
		{
			if (result)
			{
				EAE6320_ASSERT(newEffect != nullptr);
				o_effect = newEffect;
			}
			else
			{
				if (newEffect)
				{
					newEffect->DecrementReferenceCount();
					newEffect = nullptr;
				}
				o_effect = nullptr;
			}
		});

	// Allocate a new effect
	{
		newEffect = new (std::nothrow) cEffect();
		if (!newEffect)
		{
			result = Results::OutOfMemory;
			EAE6320_ASSERTF(false, "Couldn't allocate memory for the effect");
			Logging::OutputError("Failed to allocate memory for the effect");
			return result;
		}
	}

	// Initialize the platform-specific graphics API effect object
	if (!(result = newEffect->Initialize(i_vertexShader, i_fragmentShader)))
	{
		EAE6320_ASSERTF(false, "Initialization of new effect failed");
		return result;
	}

	return result;
}

// Initialize / Clean Up
//----------------------

eae6320::Graphics::cEffect::cEffect()
{
	s_effectTracker.AddEffect(this);
}

eae6320::Graphics::cEffect::~cEffect()
{
	EAE6320_ASSERT(m_referenceCount == 0);
	s_effectTracker.RemoveEffect(this);
	const auto result = CleanUp();
	EAE6320_ASSERT(result);
}

eae6320::cResult eae6320::Graphics::cEffect::InitializeData(cShader* i_vertexShader, cShader* i_fragmentShader)
{
	auto result = eae6320::Results::Success;

	if (i_vertexShader == nullptr)
	{
		result = eae6320::Results::Failure;
		return result;
	}
	else
	{
		s_vertexShader = i_vertexShader;
	}
	if (i_fragmentShader == nullptr)
	{
		result = eae6320::Results::Failure;
		return result;
	}
	else
	{
		s_fragmentShader = i_fragmentShader;
	}

	{
		constexpr auto renderStateBits = []
		{
			uint8_t renderStateBits = 0;

			eae6320::Graphics::RenderStates::DisableAlphaTransparency(renderStateBits);
			eae6320::Graphics::RenderStates::EnableDepthTesting(renderStateBits);
			eae6320::Graphics::RenderStates::EnableDepthWriting(renderStateBits);
			eae6320::Graphics::RenderStates::DisableDrawingBothTriangleSides(renderStateBits);

			return renderStateBits;
		}();
		if (!(result = s_renderState.Initialize(renderStateBits)))
		{
			EAE6320_ASSERTF(false, "Can't initialize shading data without render state");
			return result;
		}
	}

	return result;
}

// Helper Class Definition
//========================

namespace
{
	// Interface
	//----------

	// Access

	void cEffectTracker::AddEffect(const eae6320::Graphics::cEffect* const i_effect)
	{
		EAE6320_ASSERT(i_effect != nullptr);
		eae6320::Concurrency::cMutex::cScopeLock scopeLock(m_mutex);
#ifdef EAE6320_ASSERTS_AREENABLED
		for (const auto* const effect : m_effects)
		{
			EAE6320_ASSERTF(effect != i_effect, "This effect is already being tracked");
		}
#endif
		m_effects.push_back(i_effect);
	}

	void cEffectTracker::RemoveEffect(const eae6320::Graphics::cEffect* const i_effect)
	{
		eae6320::Concurrency::cMutex::cScopeLock scopeLock(m_mutex);
		const auto effectCount = m_effects.size();
		for (std::remove_const<decltype(effectCount)>::type i = 0; i < effectCount; ++i)
		{
			if (m_effects[i] == i_effect)
			{
				m_effects.erase(m_effects.begin() + i);
				return;
			}
		}
		EAE6320_ASSERTF(false, "This effect wasn't being tracked");
	}

	// Initialize / Clean Up

	cEffectTracker::~cEffectTracker()
	{
		eae6320::Concurrency::cMutex::cScopeLock scopeLock(m_mutex);
		const auto haveAnyEffectsNotBeenDeleted = !m_effects.empty();
		if (haveAnyEffectsNotBeenDeleted)
		{
			EAE6320_ASSERTF(!haveAnyEffectsNotBeenDeleted,
				"If you are seeing this message there are %i effects that haven't been deleted. You probably have a reference counting bug!", m_effects.size());
			eae6320::Logging::OutputError("Not every effect was released. The program will now crash!");
			constexpr int* const invalidAddress = 0x0;
			*invalidAddress = 123;	// Intentional crash! You have a bug that must be fixed!
		}
	}
}