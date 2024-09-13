#pragma once

#include "VertexFormats.h"
#if defined( EAE6320_PLATFORM_D3D )
#include "Direct3D/Includes.h"
#include "cVertexFormat.h"
#elif defined( EAE6320_PLATFORM_GL )
#include "OpenGL/Includes.h"
#endif

#include <Engine/Assets/ReferenceCountedAssets.h>
#include <string>

namespace eae6320
{
	namespace Graphics
	{
		class cMesh
		{
		public:
			EAE6320_ASSETS_DECLAREDELETEDREFERENCECOUNTEDFUNCTIONS( cMesh );

			EAE6320_ASSETS_DECLAREREFERENCECOUNTINGFUNCTIONS();

			static cResult Load(cMesh*& o_mesh, const std::string& i_path);
			
			void	DrawGeometry();

			EAE6320_ASSETS_DECLAREREFERENCECOUNT();
		private:
			cMesh();
			~cMesh();
			static cResult LoadMeshFromBinaryFile(const std::string& i_path, VertexFormats::sVertex_mesh* &i_vertexData, uint16_t* &i_indexData, uint16_t& i_vertexCount, uint16_t& i_indexCount);
			cResult Initialize(VertexFormats::sVertex_mesh* vertexData, uint16_t* indexData, const int vertexCount, const int indexCount);
			cResult CleanUp();

			// indexCountToRender
			int m_indexCount = 0;

#if defined( EAE6320_PLATFORM_D3D )
			eae6320::Graphics::cVertexFormat* s_vertexFormat = nullptr;
			// A vertex buffer holds the data for each vertex
			ID3D11Buffer* s_vertexBuffer = nullptr;
			// A index buffer holds the data for each index or vertex
			ID3D11Buffer* s_indexBuffer = nullptr;

#elif defined( EAE6320_PLATFORM_GL )
			// A vertex buffer holds the data for each vertex
			GLuint s_vertexBufferId = 0;
			// A vertex array encapsulates the vertex data as well as the vertex input layout
			GLuint s_vertexArrayId = 0;

			// A index buffer holds the index for each vertex
			GLuint s_indexBufferId = 0;
#endif
		};
	}
}
