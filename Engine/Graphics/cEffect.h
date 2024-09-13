#pragma once

#if defined( EAE6320_PLATFORM_D3D )
#include "Direct3D/Includes.h"
#elif defined( EAE6320_PLATFORM_GL )
#include "OpenGL/Includes.h"
#endif
#include "cShader.h"
#include "cRenderState.h"

#include <Engine/Results/Results.h>

namespace eae6320
{
	namespace Graphics
	{
		class cEffect
		{
		public:
			EAE6320_ASSETS_DECLAREDELETEDREFERENCECOUNTEDFUNCTIONS(cEffect);

			EAE6320_ASSETS_DECLAREREFERENCECOUNTINGFUNCTIONS();

			static cResult Load(cEffect*& o_effect, cShader* i_vertexShader, cShader* i_fragmentShader);

			void Bind();

			EAE6320_ASSETS_DECLAREREFERENCECOUNT();
		private:
			cEffect();
			~cEffect();

			cResult InitializeData(cShader* i_vertexShader, cShader* i_fragmentShader);
			cResult Initialize(cShader* i_vertexShader, cShader* i_fragmentShader);
			cResult CleanUp();

			eae6320::Graphics::cShader* s_vertexShader = nullptr;
			eae6320::Graphics::cShader* s_fragmentShader = nullptr;
			eae6320::Graphics::cRenderState s_renderState;
#if defined( EAE6320_PLATFORM_GL )
			GLuint s_programId = 0;
#endif
		};
	}
}