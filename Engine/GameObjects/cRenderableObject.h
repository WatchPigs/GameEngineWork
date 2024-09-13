#pragma once

#include <Engine/Physics/sRigidBodyState.h>
#include <Engine/Assets/ReferenceCountedAssets.h>
#include <Engine/Results/Results.h>
#include <Engine/Math/cMatrix_transformation.h>
#include <Engine/Math/cQuaternion.h>
#include <Engine/Math/sVector.h>
#include <Engine/Graphics/cEffect.h>
#include <Engine/Graphics/cMesh.h>
#include <Engine/Graphics/Graphics.h>

namespace eae6320
{
	namespace GameObjects
	{
		class cRenderableObject
		{
		public:
			EAE6320_ASSETS_DECLAREDELETEDREFERENCECOUNTEDFUNCTIONS(cRenderableObject);

			EAE6320_ASSETS_DECLAREREFERENCECOUNTINGFUNCTIONS();

			static cResult Load(cRenderableObject*& o_renderableObject, const Physics::sRigidBodyState i_rigidBodyState, Graphics::cEffect** i_ppEffect, unsigned int i_effctsCount, Graphics::cMesh** i_ppMesh, unsigned int i_meshesCount);

			EAE6320_ASSETS_DECLAREREFERENCECOUNT();

			void UpdateSimulation(const float i_secondCountToIntegrate);

			Math::cMatrix_transformation PredictFutureTransform(const float i_secondCountToExtrapolate);

			void SetVelocity(Math::sVector i_velocity);

			void SetPosition(Math::sVector i_position);

			void SetEffectIndex(const unsigned int i_effectIndex);

			void SetMeshIndex(const unsigned int i_meshIndex);

			Graphics::sEffectDrawCallAndMesh GetRenderData(const float i_elapsedSecondCount_sinceLastSimulationUpdate);

		private:
			cRenderableObject();
			~cRenderableObject();

			cResult Initialize(const Physics::sRigidBodyState i_rigidBodyState, Graphics::cEffect** i_ppEffect, unsigned int i_effctsCount, Graphics::cMesh** i_ppMesh, unsigned int i_meshesCount);
			cResult CleanUp();

			Physics::sRigidBodyState m_rigidBodyState;

			unsigned int m_effctsCount = 0;
			unsigned int m_meshesCount = 0;

			Graphics::cEffect** m_ppEffect = nullptr;
			Graphics::cMesh** m_ppMesh = nullptr;

			unsigned int m_effectIndex = 0;
			unsigned int m_meshIndex = 0;
		};
	}
}
