#pragma once

#include <Engine/Physics/sRigidBodyState.h>
#include <Engine/Assets/ReferenceCountedAssets.h>
#include <Engine/Results/Results.h>
#include <Engine/Math/cMatrix_transformation.h>
#include <Engine/Math/cQuaternion.h>
#include <Engine/Math/sVector.h>

namespace eae6320
{
	namespace GameObjects
	{
		class cCamera
		{
		public:
			EAE6320_ASSETS_DECLAREDELETEDREFERENCECOUNTEDFUNCTIONS(cCamera);

			EAE6320_ASSETS_DECLAREREFERENCECOUNTINGFUNCTIONS();

			static cResult Load(cCamera*& o_camera, const Physics::sRigidBodyState i_rigidBodyState, Math::cMatrix_transformation i_transform_cameraToProjected);

			EAE6320_ASSETS_DECLAREREFERENCECOUNT();

			void UpdateSimulation(const float i_secondCountToIntegrate);

			void PredictForRender(const float i_secondCountToExtrapolate);

			void SetVelocity(Math::sVector i_velocity);

			Math::cMatrix_transformation GetTransformWorldToCamera();
			Math::cMatrix_transformation GetTransformCameraToProjected();

		private:
			cCamera();
			~cCamera();

			cResult Initialize(const Physics::sRigidBodyState i_rigidBodyState, const Math::cMatrix_transformation i_transform_cameraToProjected);
			cResult CleanUp();

			Physics::sRigidBodyState m_rigidBodyState;

			Math::cMatrix_transformation m_transform_worldToCamera;
			Math::cMatrix_transformation m_transform_cameraToProjected;
		};
	}
}

