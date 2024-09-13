#pragma once

#if defined( EAE6320_PLATFORM_D3D )
#include "Direct3D/Includes.h"
#elif defined( EAE6320_PLATFORM_GL )
#include "OpenGL/Includes.h"
#endif

#include "Graphics.h"

#include <Engine/Assets/ReferenceCountedAssets.h>
#include <Engine/Results/Results.h>

namespace eae6320
{
	namespace Graphics
	{
		class cRenderTarget
		{
		public:
			EAE6320_ASSETS_DECLAREDELETEDREFERENCECOUNTEDFUNCTIONS(cRenderTarget);

			EAE6320_ASSETS_DECLAREREFERENCECOUNTINGFUNCTIONS();

			static cResult Load(cRenderTarget*& o_renderTarget, const sInitializationParameters& i_initializationParameters);

			void ClearBuffer(float* i_clearColor);

			void SwapBuffer();

			EAE6320_ASSETS_DECLAREREFERENCECOUNT();

		private:
			cRenderTarget();
			~cRenderTarget();
			
			cResult Initialize(const sInitializationParameters& i_initializationParameters);
			cResult CleanUp();
#if defined( EAE6320_PLATFORM_D3D )
			cResult InitializeViews(const unsigned int i_resolutionWidth, const unsigned int i_resolutionHeight);

			// In Direct3D "views" are objects that allow a texture to be used a particular way:
			// A render target view allows a texture to have color rendered to it
			ID3D11RenderTargetView* s_renderTargetView = nullptr;
			// A depth/stencil view allows a texture to have depth rendered to it
			ID3D11DepthStencilView* s_depthStencilView = nullptr;
#endif
		};
	}
}

