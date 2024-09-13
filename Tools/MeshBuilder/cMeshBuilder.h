/*EAE6320_CSHADERBUILDER_H
	This class builds meshs
*/

#ifndef EAE6320_CSHADERBUILDER_H
#define EAE6320_CSHADERBUILDER_H

// Includes
//=========

#include <Tools/AssetBuildLibrary/iBuilder.h>

#include <Engine/Graphics/Configuration.h>
#include <Engine/Graphics/VertexFormats.h>
#include <External/Lua/Includes.h>

// Class Declaration
//==================

namespace eae6320
{
	namespace Assets
	{
		class cMeshBuilder final : public iBuilder
		{
			// Inherited Implementation
			//=========================

		private:

			// Build
			//------

			cResult Build(const std::vector<std::string>& i_arguments) final;


			eae6320::cResult LuaToBinary(const char* const i_path_source, const char* const i_path_target);
			eae6320::cResult LoadMeshFromFile(const std::string& i_path, eae6320::Graphics::VertexFormats::sVertex_mesh*& i_vertexData, uint16_t*& i_indexData, uint16_t& i_vertexCount, uint16_t& i_indexCount);
			eae6320::cResult LoadVertex(lua_State& io_luaState, eae6320::Graphics::VertexFormats::sVertex_mesh*& i_vertexData, uint16_t& i_vertexCount);
			eae6320::cResult LoadIndex(lua_State& io_luaState, uint16_t*& i_indexData, uint16_t& i_indexCount);
		};
	}
}

#endif	// EAE6320_CSHADERBUILDER_H