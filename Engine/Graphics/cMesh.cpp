// Includes
//=========

#include "cMesh.h"

#include <Engine/Asserts/Asserts.h>
#include <Engine/Concurrency/cMutex.h>
#include <Engine/Logging/Logging.h>
#include <Engine/ScopeGuard/cScopeGuard.h>
#include <External/Lua/Includes.h>
#include <Engine/Platform/Platform.h>
#include <Engine/Time/Time.h>
#include <new>
#include <vector>
#include <fstream>

namespace
{
	// This exists for our class to help you catch reference counting problems.
	// In a real game there would be more sophisticated methods that would work for all asset types.
	class cMeshTracker
	{
		// Interface
		//----------

	public:

		// Access
		void AddMesh(const eae6320::Graphics::cMesh* const i_mesh);
		void RemoveMesh(const eae6320::Graphics::cMesh* const i_mesh);
		// Initializate / Clean Up
		cMeshTracker() = default;
		~cMeshTracker();

		// Data
		//-----

	private:

		std::vector<const eae6320::Graphics::cMesh*> m_meshs;
		eae6320::Concurrency::cMutex m_mutex;

		// Implementation
		//---------------

	private:

		// Initializate / Clean Up
		cMeshTracker(const cMeshTracker&) = delete;
		cMeshTracker(cMeshTracker&&) = delete;
		cMeshTracker& operator =(const cMeshTracker&) = delete;
		cMeshTracker& operator =(cMeshTracker&&) = delete;
	} s_meshTracker;
}


eae6320::cResult eae6320::Graphics::cMesh::Load(cMesh*& o_mesh, const std::string& i_path)
{
	auto result = Results::Success;

	cMesh* newMesh = nullptr;
	cScopeGuard scopeGuard([&o_mesh, &result, &newMesh]
		{
			if (result)
			{
				EAE6320_ASSERT(newMesh != nullptr);
				o_mesh = newMesh;
			}
			else
			{
				if (newMesh)
				{
					newMesh->DecrementReferenceCount();
					newMesh = nullptr;
				}
				o_mesh = nullptr;
			}
		});

	// Allocate a new mesh
	{
		newMesh = new (std::nothrow) cMesh();
		if (!newMesh)
		{
			result = Results::OutOfMemory;
			EAE6320_ASSERTF(false, "Couldn't allocate memory for the mesh");
			Logging::OutputError("Failed to allocate memory for the mesh");
			return result;
		}
	}

	//Load mesh data from lua file
	VertexFormats::sVertex_mesh* vertexData = nullptr;
	uint16_t* indexData = nullptr;
	uint16_t vertexCount = 0;
	uint16_t indexCount = 0;
	if (!(result = LoadMeshFromBinaryFile(i_path, vertexData, indexData, vertexCount, indexCount)))
	{
		EAE6320_ASSERTF(false, "Load mesh data failed");
		return result;
	}

	// Initialize the platform-specific graphics API mesh object
	if (!(result = newMesh->Initialize(vertexData, indexData, static_cast<int>(vertexCount), static_cast<int>(indexCount))))
	{
		EAE6320_ASSERTF(false, "Initialization of new mesh failed");
		return result;
	}
	delete[] vertexData;
	delete[] indexData;
	vertexData = nullptr;
	indexData = nullptr;

	// set indexCount
	newMesh->m_indexCount = indexCount;

	return result;
}

// Implementation
//===============

// Initialize / Clean Up
//----------------------

eae6320::Graphics::cMesh::cMesh()
{
	s_meshTracker.AddMesh(this);
}

eae6320::Graphics::cMesh::~cMesh()
{
	EAE6320_ASSERT(m_referenceCount == 0);
	s_meshTracker.RemoveMesh(this);
	const auto result = CleanUp();
	EAE6320_ASSERT(result);
}

eae6320::cResult eae6320::Graphics::cMesh::LoadMeshFromBinaryFile(const std::string& i_path, VertexFormats::sVertex_mesh*& i_vertexData, uint16_t*& i_indexData, uint16_t& i_vertexCount, uint16_t& i_indexCount)
{
	auto result = eae6320::Results::Success;

	eae6320::Platform::sDataFromFile dataFromFile;
	eae6320::Platform::LoadBinaryFile(i_path.c_str(), dataFromFile);

	auto currentOffset = reinterpret_cast<uintptr_t>(dataFromFile.data);
	const auto finalOffset = currentOffset + dataFromFile.size;
	memcpy(&i_vertexCount, reinterpret_cast<void*>(currentOffset), sizeof(i_vertexCount));
	currentOffset += sizeof(i_vertexCount);
	memcpy(&i_indexCount, reinterpret_cast<void*>(currentOffset), sizeof(i_indexCount));
	currentOffset += sizeof(i_indexCount);

	size_t targetSize = sizeof(i_vertexCount) + sizeof(i_indexCount) + sizeof(VertexFormats::sVertex_mesh) * i_vertexCount + sizeof(uint16_t) * i_indexCount;
	EAE6320_ASSERTF(dataFromFile.size == targetSize, "dataFromFile size is not correct.");

	i_vertexData = new VertexFormats::sVertex_mesh[i_vertexCount];
	memcpy(i_vertexData, reinterpret_cast<void*>(currentOffset), sizeof(VertexFormats::sVertex_mesh) * i_vertexCount);
	i_indexData = new uint16_t[i_indexCount];
	currentOffset += sizeof(VertexFormats::sVertex_mesh) * i_vertexCount;
	memcpy(i_indexData, reinterpret_cast<void*>(currentOffset), sizeof(uint16_t) * i_indexCount);
	currentOffset += sizeof(uint16_t) * i_indexCount;

	return result;
}


// Helper Class Definition
//========================

namespace
{
	// Interface
	//----------

	// Access

	void cMeshTracker::AddMesh(const eae6320::Graphics::cMesh* const i_mesh)
	{
		EAE6320_ASSERT(i_mesh != nullptr);
		eae6320::Concurrency::cMutex::cScopeLock scopeLock(m_mutex);
#ifdef EAE6320_ASSERTS_AREENABLED
		for (const auto* const mesh : m_meshs)
		{
			EAE6320_ASSERTF(mesh != i_mesh, "This mesh is already being tracked");
		}
#endif
		m_meshs.push_back(i_mesh);
	}

	void cMeshTracker::RemoveMesh(const eae6320::Graphics::cMesh* const i_mesh)
	{
		eae6320::Concurrency::cMutex::cScopeLock scopeLock(m_mutex);
		const auto meshCount = m_meshs.size();
		for (std::remove_const<decltype(meshCount)>::type i = 0; i < meshCount; ++i)
		{
			if (m_meshs[i] == i_mesh)
			{
				m_meshs.erase(m_meshs.begin() + i);
				return;
			}
		}
		EAE6320_ASSERTF(false, "This mesh wasn't being tracked");
	}

	// Initialize / Clean Up

	cMeshTracker::~cMeshTracker()
	{
		eae6320::Concurrency::cMutex::cScopeLock scopeLock(m_mutex);
		const auto haveAnyMeshsNotBeenDeleted = !m_meshs.empty();
		if (haveAnyMeshsNotBeenDeleted)
		{
			EAE6320_ASSERTF(!haveAnyMeshsNotBeenDeleted,
				"If you are seeing this message there are %i meshs that haven't been deleted. You probably have a reference counting bug!", m_meshs.size());
			eae6320::Logging::OutputError("Not every mesh was released. The program will now crash!");
			constexpr int* const invalidAddress = 0x0;
			*invalidAddress = 123;	// Intentional crash! You have a bug that must be fixed!
		}
	}
}
