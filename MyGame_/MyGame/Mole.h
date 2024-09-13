#pragma once

#include <Engine/Graphics/cMesh.h>
#include <Engine/Graphics/cEffect.h>
#include <Engine/GameObjects/cRenderableObject.h>
#include <Engine/Results/Results.h>
#include <Engine/Math/sVector.h>

namespace eae6320
{
	class Mole
	{
	public:
		bool GetActive();
		void SetActive(bool i_active);
		eae6320::GameObjects::cRenderableObject* GetRenderableObject();
		void Initialize();
		void CleanUp();
		void SetPosition(Math::sVector i_position);
	private:
		bool m_active = false;
		bool m_initialized = false;
		eae6320::GameObjects::cRenderableObject* m_renderableObject;
	};
}


