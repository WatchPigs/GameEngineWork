#include "cMeshBuilder.h"
#include <Tools/AssetBuildLibrary/Functions.h>
#include <Engine/Graphics/VertexFormats.h>
#include <Engine/Platform/Platform.h>
#include <Engine/ScopeGuard/cScopeGuard.h>
#include <Engine/Asserts/Asserts.h>
#include <Engine/Time/Time.h>
#include <fstream>

eae6320::cResult eae6320::Assets::cMeshBuilder::Build(const std::vector<std::string>& i_arguments)
{
	auto result = Results::Success;

	std::string* errorMessage = nullptr;

	//result = eae6320::Platform::CopyFile(m_path_source, m_path_target, false, true, errorMessage);
	result = LuaToBinary(m_path_source, m_path_target);

	if (!result.IsSuccess() && errorMessage != nullptr) OutputErrorMessageWithFileInfo(m_path_source, errorMessage->c_str());

	return result;
}

eae6320::cResult eae6320::Assets::cMeshBuilder::LuaToBinary(const char* const i_path_source, const char* const i_path_target)
{
	auto result = Results::Success;

	//Load mesh data from lua file
	eae6320::Graphics::VertexFormats::sVertex_mesh* vertexData = nullptr;
	uint16_t* indexData = nullptr;
	uint16_t vertexCount = 0;
	uint16_t indexCount = 0;

	if (!(result = LoadMeshFromFile(i_path_source, vertexData, indexData, vertexCount, indexCount)))
	{
		//EAE6320_ASSERTF(false, "Load mesh data failed");
		OutputErrorMessageWithFileInfo(m_path_source, "Load mesh data failed");
		return result;
	}

	std::ofstream targetFile(i_path_target, std::ios::out | std::ios::binary);

	targetFile.write(reinterpret_cast<char*>(&vertexCount), sizeof(vertexCount));
	targetFile.write(reinterpret_cast<char*>(&indexCount), sizeof(indexCount));
	targetFile.write(reinterpret_cast<char*>(vertexData), sizeof(*vertexData) * static_cast<uint64_t>(vertexCount));
	targetFile.write(reinterpret_cast<char*>(indexData), sizeof(*indexData) * static_cast<uint64_t>(indexCount));

	targetFile.close();

	delete[] vertexData;
	delete[] indexData;
	vertexData = nullptr;
	indexData = nullptr;

	return result;
}

eae6320::cResult eae6320::Assets::cMeshBuilder::LoadMeshFromFile(const std::string& i_path, eae6320::Graphics::VertexFormats::sVertex_mesh*& i_vertexData, uint16_t*& i_indexData, uint16_t& i_vertexCount, uint16_t& i_indexCount)
{
	auto result = eae6320::Results::Success;

	// Create a new Lua state
	lua_State* luaState = nullptr;
	eae6320::cScopeGuard scopeGuard_onExit([&luaState]
		{
			if (luaState)
			{
				// If I haven't made any mistakes
				// there shouldn't be anything on the stack
				// regardless of any errors
				EAE6320_ASSERT(lua_gettop(luaState) == 0);

				lua_close(luaState);
				luaState = nullptr;
			}
		});
	{
		luaState = luaL_newstate();
		if (!luaState)
		{
			result = eae6320::Results::OutOfMemory;
			//EAE6320_ASSERTF(false, "Failed to create a new Lua state");
			OutputErrorMessageWithFileInfo(m_path_source, "Failed to create a new Lua state");
			return result;
		}
	}

	// Load the asset file as a "chunk",
	// meaning there will be a callable function at the top of the stack
	const auto stackTopBeforeLoad = lua_gettop(luaState);
	{
		const auto luaResult = luaL_loadfile(luaState, i_path.c_str());
		if (luaResult != LUA_OK)
		{
			result = eae6320::Results::Failure;
			//EAE6320_ASSERTF(false, lua_tostring(luaState, -1));
			OutputErrorMessageWithFileInfo(m_path_source, lua_tostring(luaState, -1));
			// Pop the error message
			lua_pop(luaState, 1);
			return result;
		}
	}
	// Execute the "chunk", which should load the asset
	// into a table at the top of the stack
	{
		constexpr int argumentCount = 0;
		constexpr int returnValueCount = LUA_MULTRET;	// Return _everything_ that the file returns
		constexpr int noMessageHandler = 0;
		const auto luaResult = lua_pcall(luaState, argumentCount, returnValueCount, noMessageHandler);
		if (luaResult == LUA_OK)
		{
			// A well-behaved asset file will only return a single value
			const auto returnedValueCount = lua_gettop(luaState) - stackTopBeforeLoad;
			if (returnedValueCount == 1)
			{
				// A correct asset file _must_ return a table
				if (!lua_istable(luaState, -1))
				{
					result = eae6320::Results::InvalidFile;
					const char* str0 = "Asset files must return a table (instead of a ";
					const char* str1 = luaL_typename(luaState, -1);
					const char* str2 = ")";
					char* str = new char[strlen(str0) + strlen(str1) + strlen(str2) + 1];
					strcpy(str, str0);
					strcat(str, str1);
					strcat(str, str2);
					//EAE6320_ASSERTF(false, str);
					OutputErrorMessageWithFileInfo(m_path_source, str);
					delete[] str;
					// Pop the returned non-table value
					lua_pop(luaState, 1);
					return result;
				}
			}
			else
			{
				result = eae6320::Results::InvalidFile;
				const char* str0 = "Asset files must return a single table (instead of ";
				const char* str2 = " values)";
				char* str = new char[strlen(str0) + strlen(std::to_string(returnedValueCount).c_str()) + strlen(str2) + 1];
				strcpy(str, str0);
				strcat(str, std::to_string(returnedValueCount).c_str());
				strcat(str, str2);
				//EAE6320_ASSERTF(false, str);
				OutputErrorMessageWithFileInfo(m_path_source, str);
				delete[] str;
				// Pop every value that was returned
				lua_pop(luaState, returnedValueCount);
				return result;
			}
		}
		else
		{
			result = eae6320::Results::InvalidFile;
			//EAE6320_ASSERTF(false, lua_tostring(luaState, -1));
			OutputErrorMessageWithFileInfo(m_path_source, lua_tostring(luaState, -1));
			// Pop the error message
			lua_pop(luaState, 1);
			return result;
		}
	}

	// If this code is reached the asset file was loaded successfully,
	// and its table is now at index -1
	eae6320::cScopeGuard scopeGuard_popAssetTable([luaState]
		{
			lua_pop(luaState, 1);
		});

	// Load Vertex
	{
		// Right now the asset table is at -1.
		// After the following table operation it will be at -2
		// and the "vertexData" table will be at -1:
		constexpr auto* const key = "vertexData";
		lua_pushstring(luaState, key);
		lua_gettable(luaState, -2);
		// It can be hard to remember where the stack is at
		// and how many values to pop.
		// There are two ways that I suggest making it easier to keep track of this:
		//	1) Use scope guards to pop things automatically
		//	2) Call a different function when you are at a new level
		// Right now we know that we have an original table at -2,
		// and a new one at -1,
		// and so we _know_ that we always have to pop at least _one_
		// value before leaving this function
		// (to make the original table be back to index -1).
		// We can create a scope guard immediately as soon as the new table has been pushed
		// to guarantee that it will be popped when we are done with it:
		eae6320::cScopeGuard scopeGuard_popVertexData([luaState]
			{
				lua_pop(luaState, 1);
			});
		// Additionally, I try not to do any further stack manipulation in this function
		// and call other functions that assume the "vertexData" table is at -1
		// but don't know or care about the rest of the stack
		if (lua_istable(luaState, -1))
		{
			if (!(result = LoadVertex(*luaState, i_vertexData, i_vertexCount)))
			{
				return result;
			}
		}
		else
		{
			result = eae6320::Results::InvalidFile;
			const char* str0 = "The value at \"";
			const char* str1 = key;
			const char* str2 = "\" must be a table (instead of a ";
			const char* str3 = luaL_typename(luaState, -1);
			const char* str4 = ")";
			char* str = new char[strlen(str0) + strlen(str1) + strlen(str2) + strlen(str3) + strlen(str4) + 1];
			strcpy(str, str0);
			strcat(str, str1);
			strcat(str, str2);
			strcat(str, str3);
			strcat(str, str4);
			//EAE6320_ASSERTF(false, str);
			OutputErrorMessageWithFileInfo(m_path_source, str);
			delete[] str;
			return result;
		}
	}
	// Load Index
	{
		// Right now the asset table is at -1.
		// After the following table operation it will be at -2
		// and the "indexData" table will be at -1:
		constexpr auto* const key = "indexData";
		lua_pushstring(luaState, key);
		lua_gettable(luaState, -2);
		eae6320::cScopeGuard scopeGuard_popindexData([luaState]
			{
				lua_pop(luaState, 1);
			});
		if (lua_istable(luaState, -1))
		{
			if (!(result = LoadIndex(*luaState, i_indexData, i_indexCount)))
			{
				return result;
			}
		}
		else
		{
			result = eae6320::Results::InvalidFile;
			const char* str0 = "The value at \"";
			const char* str1 = key;
			const char* str2 = "\" must be a table (instead of a ";
			const char* str3 = luaL_typename(luaState, -1);
			const char* str4 = ")";
			char* str = new char[strlen(str0) + strlen(str1) + strlen(str2) + strlen(str3) + strlen(str4) + 1];
			strcpy(str, str0);
			strcat(str, str1);
			strcat(str, str2);
			strcat(str, str3);
			strcat(str, str4);
			//EAE6320_ASSERTF(false, str);
			OutputErrorMessageWithFileInfo(m_path_source, str);
			delete[] str;
			return result;
		}
	}


	return result;
}

//Load Vertex from lua
eae6320::cResult eae6320::Assets::cMeshBuilder::LoadVertex(lua_State& io_luaState, eae6320::Graphics::VertexFormats::sVertex_mesh*& i_vertexData, uint16_t& i_vertexCount)
{
	auto result = eae6320::Results::Success;
	const auto vertexCount = static_cast<int>(luaL_len(&io_luaState, -1));
	i_vertexCount = vertexCount;
	i_vertexData = new eae6320::Graphics::VertexFormats::sVertex_mesh[vertexCount];
	for (int i = 1; i <= vertexCount; ++i)
	{
		lua_pushinteger(&io_luaState, i);
		lua_gettable(&io_luaState, -2);
		eae6320::cScopeGuard scopeGuard_popVertexArray([&io_luaState]
			{
				lua_pop(&io_luaState, 1);
			});
		{
			constexpr auto* const key = "position";
			lua_pushstring(&io_luaState, key);
			lua_gettable(&io_luaState, -2);
			eae6320::cScopeGuard scopeGuard_popPosition([&io_luaState]
				{
					lua_pop(&io_luaState, 1);
				});
			if (lua_istable(&io_luaState, -1))
			{
				for (int j = 1; j <= 3; ++j)
				{
					lua_pushinteger(&io_luaState, j);
					lua_gettable(&io_luaState, -2);
					eae6320::cScopeGuard scopeGuard_popPositionArray([&io_luaState]
						{
							lua_pop(&io_luaState, 1);
						});
					switch (j)
					{
					case 1:
						i_vertexData[i - 1].x = static_cast<float>(lua_tonumber(&io_luaState, -1));
						break;
					case 2:
						i_vertexData[i - 1].y = static_cast<float>(lua_tonumber(&io_luaState, -1));
						break;
					case 3:
						i_vertexData[i - 1].z = static_cast<float>(lua_tonumber(&io_luaState, -1));
						break;
					}
				}
			}
			else
			{
				result = eae6320::Results::InvalidFile;
				const char* str0 = "The value at \"";
				const char* str1 = key;
				const char* str2 = "\" must be a table (instead of a ";
				const char* str3 = luaL_typename(&io_luaState, -1);
				const char* str4 = ")";
				char* str = new char[strlen(str0) + strlen(str1) + strlen(str2) + strlen(str3) + strlen(str4) + 1];
				strcpy(str, str0);
				strcat(str, str1);
				strcat(str, str2);
				strcat(str, str3);
				strcat(str, str4);
				//EAE6320_ASSERTF(false, str);
				OutputErrorMessageWithFileInfo(m_path_source, str);
				delete[] str;
				return result;
			}
		}
	}
	return result;
}

//Loda index from lua
eae6320::cResult eae6320::Assets::cMeshBuilder::LoadIndex(lua_State& io_luaState, uint16_t*& i_indexData, uint16_t& i_indexCount)
{
	auto result = eae6320::Results::Success;
	const auto indexCount = static_cast<int>(luaL_len(&io_luaState, -1));
	i_indexCount = indexCount;
	i_indexData = new uint16_t[indexCount];
	for (int i = 1; i <= indexCount; ++i)
	{
		lua_pushinteger(&io_luaState, i);
		lua_gettable(&io_luaState, -2);
		eae6320::cScopeGuard scopeGuard_popIndexArray([&io_luaState]
			{
				lua_pop(&io_luaState, 1);
			});
		i_indexData[i - 1] = static_cast<uint16_t>(lua_tonumber(&io_luaState, -1));
	}
	return result;
}