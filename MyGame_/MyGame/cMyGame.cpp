// Includes
//=========

#include "cMyGame.h"

#include <Engine/Graphics/cMesh.h>
#include <Engine/Graphics/cEffect.h>
#include <Engine/Asserts/Asserts.h>
#include <Engine/UserInput/UserInput.h>
#include <Engine/UserInput/Mouse.h>
#include <Engine/Logging/Logging.h>
#include <Engine/Graphics/Graphics.h>
#include <Engine/Physics/sRigidBodyState.h>
#include <Engine/Math/cMatrix_transformation.h>
#include <Engine/Math/cQuaternion.h>
#include <Engine/Math/sVector.h>
#include <Engine/Math/Functions.h>

#include <vector>
#include <Engine/Math/sVector.h>
#include <ctime>
#include <fstream>

// Inherited Implementation
//=========================

// Run
//----

void eae6320::cMyGame::UpdateBasedOnInput()
{
	// Is the user pressing the ESC key?
	if (UserInput::IsKeyPressed(UserInput::KeyCodes::Escape))
	{
		const auto result = Exit(EXIT_SUCCESS);
		EAE6320_ASSERT(result);
	}
}

void eae6320::cMyGame::LoadGame()
{
	std::ifstream gameSaveFile(EAE6320_GAMESAVE_PATH);
	std::ostringstream tmp;
	tmp << gameSaveFile.rdbuf();
	string gameSaveJson = tmp.str();
	gameSaveFile.close();
	Serializable::Regist<GameSave>();
	bool over = false;
	GameSave gameSave;
	try
	{
		gameSave = Serializable::loads<GameSave>(gameSaveJson);
	}
	catch (std::exception)
	{
		over = true;
	}

	if (!over) over = gameSave.over;

	if (over)
	{
		for (int i = 0; i < 9; i++) areas[i] = true;

		srand(static_cast<unsigned int>(time(0)));
		mine = rand() % 9;
		step = 0;
	}
	else
	{
		mine = gameSave.mine;
		step = gameSave.step;
		memcpy(areas, gameSave.areas, 9);
	}

}

void eae6320::cMyGame::SaveGame()
{
	void* gameSave = Reflectable::get_instance("struct GameSave");
	if (gameOver)
	{
		Reflectable::set_field<bool>(gameSave, "struct GameSave", "over", true);
	}
	else
	{
		Reflectable::set_field<bool>(gameSave, "struct GameSave", "over", false);
		Reflectable::set_field<int>(gameSave, "struct GameSave", "mine", mine);
		Reflectable::set_field<int>(gameSave, "struct GameSave", "step", step);
		bool* gameSave_areas = Reflectable::get_field<bool*>(gameSave, "struct GameSave", "areas");
		for (int i = 0; i < 9; i++)
		{
			(*(GameSave*)gameSave).areas[i] = areas[i];
		}
	}
	std::string gameSaveJson = Serializable::dumps(*(GameSave*)gameSave);
	std::ofstream gameSaveFile;
	gameSaveFile.open(EAE6320_GAMESAVE_PATH, 'w');
	gameSaveFile << gameSaveJson;
	gameSaveFile.close();
}

// Initialize / Clean Up
//----------------------

eae6320::cResult eae6320::cMyGame::Initialize()
{
	eae6320::Logging::OutputMessage("Initialize Yexiang's EAE6320 My Game.");

	LoadGame();

	InitializeGameObjects();

	CreateCameras();

	return Results::Success;
}

eae6320::cResult eae6320::cMyGame::CleanUp()
{
	SaveGame();

	eae6320::Logging::OutputMessage("CleanUp Yexiang's EAE6320 My Game.");

	CleanUpGameObjects();

	CleanUpCameras();

	return Results::Success;
}

//Submit render data
void eae6320::cMyGame::SubmitDataToBeRendered(const float i_elapsedSecondCount_systemTime, const float i_elapsedSecondCount_sinceLastSimulationUpdate)
{
	float clearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
	Graphics::SubmitClearColor(clearColor);

	constexpr int renderDataCount = 9;
	Graphics::sEffectDrawCallAndMesh* renderData = new Graphics::sEffectDrawCallAndMesh[renderDataCount];
	for (int i = 0; i < 9; i++) renderData[i] = m_renderableObjects[i]->GetRenderData(i_elapsedSecondCount_sinceLastSimulationUpdate);

	Graphics::SubmitEffectsDrawCallsAndMeshes(renderData, renderDataCount);

	m_camera_0->PredictForRender(i_elapsedSecondCount_sinceLastSimulationUpdate);
	Graphics::SubmitCamera(m_camera_0->GetTransformWorldToCamera(), m_camera_0->GetTransformCameraToProjected());
}

void eae6320::cMyGame::UpdateSimulationBasedOnInput()
{
	eae6320::UserInput::Mouse mouse = eae6320::UserInput::GetMouse();

	while (!mouse.EventBufferIsEmpty())
	{
		eae6320::UserInput::MouseEvent me = mouse.ReadEvent();

		if (me.GetType() == eae6320::UserInput::MouseEvent::EventType::RPress && mouse.IsRightDown())
		{
			int area = ConvertToArea(mouse.GetPosX(), mouse.GetPosY());
			if (area >= 0 && areas[area] && (!gameOver))
			{
				areas[area] = false;
				step++;
				if (step >= 9 || area == mine)
				{
					m_renderableObjects[area]->SetEffectIndex(2);
					gameOver = true;
				}
				else
				{
					m_renderableObjects[area]->SetEffectIndex(0);
				}
			}
		}
	}
}

void eae6320::cMyGame::UpdateSimulationBasedOnTime(const float i_elapsedSecondCount_sinceLastUpdate)
{
	m_camera_0->UpdateSimulation(i_elapsedSecondCount_sinceLastUpdate);
	for (int i = 0; i < 9; i++) m_renderableObjects[i]->UpdateSimulation(i_elapsedSecondCount_sinceLastUpdate);
}

void eae6320::cMyGame::CreateCameras()
{
	Physics::sRigidBodyState cameraRigidBodyState;
	cameraRigidBodyState.position = Math::sVector(0.0f, 0.0f,3.67f);
	cameraRigidBodyState.orientation = Math::cQuaternion();
	cameraRigidBodyState.velocity = Math::sVector(0.0f, 0.0f, 0.0f);
	cameraRigidBodyState.acceleration = Math::sVector(0.0f, 0.0f, 0.0f);

	Math::cMatrix_transformation transform_cameraToProjected = Math::cMatrix_transformation::CreateCameraToProjectedTransform_perspective(Math::ConvertDegreesToRadians(45.0f), 1.0f, 0.1f, 10.1f);

	if (!eae6320::GameObjects::cCamera::Load(m_camera_0, cameraRigidBodyState, transform_cameraToProjected))
	{
		EAE6320_ASSERTF(false, "Can't initialize camera");
	}
}

void eae6320::cMyGame::CleanUpCameras()
{
	if (m_camera_0 != nullptr)
	{
		m_camera_0->DecrementReferenceCount();
		m_camera_0 = nullptr;
	}
}

void eae6320::cMyGame::InitializeGameObjects()
{
	Physics::sRigidBodyState gameObjectRigidBodyState;
	{
		gameObjectRigidBodyState.position = Math::sVector(0.0f, 0.0f, 0.0f);
		gameObjectRigidBodyState.orientation = Math::cQuaternion();
		gameObjectRigidBodyState.velocity = Math::sVector(0.0f, 0.0f, 0.0f);
		gameObjectRigidBodyState.acceleration = Math::sVector(0.0f, 0.0f, 0.0f);
	}
	eae6320::Graphics::cEffect* activeEffect;
	{
		eae6320::Graphics::cShader* vertexShader;
		eae6320::Graphics::cShader* fragmentShader;
		if (!eae6320::Graphics::cShader::Load("data/Shaders/Vertex/standard.shader",
			vertexShader, eae6320::Graphics::eShaderType::Vertex))
		{
			EAE6320_ASSERTF(false, "Can't initialize shading data without vertex shader");
		}
		if (!eae6320::Graphics::cShader::Load("data/Shaders/Fragment/MoleActive.shader",
			fragmentShader, eae6320::Graphics::eShaderType::Fragment))
		{
			EAE6320_ASSERTF(false, "Can't initialize shading data without fragment shader");
		}
		eae6320::Graphics::cEffect::Load(activeEffect, vertexShader, fragmentShader);
	}
	eae6320::Graphics::cEffect* inactiveEffect;
	{
		eae6320::Graphics::cShader* vertexShader;
		eae6320::Graphics::cShader* fragmentShader;
		if (!eae6320::Graphics::cShader::Load("data/Shaders/Vertex/standard.shader",
			vertexShader, eae6320::Graphics::eShaderType::Vertex))
		{
			EAE6320_ASSERTF(false, "Can't initialize shading data without vertex shader");
		}
		if (!eae6320::Graphics::cShader::Load("data/Shaders/Fragment/MoleInactive.shader",
			fragmentShader, eae6320::Graphics::eShaderType::Fragment))
		{
			EAE6320_ASSERTF(false, "Can't initialize shading data without fragment shader");
		}
		eae6320::Graphics::cEffect::Load(inactiveEffect, vertexShader, fragmentShader);
	}
	eae6320::Graphics::cEffect* endEffect;
	{
		eae6320::Graphics::cShader* vertexShader;
		eae6320::Graphics::cShader* fragmentShader;
		if (!eae6320::Graphics::cShader::Load("data/Shaders/Vertex/standard.shader",
			vertexShader, eae6320::Graphics::eShaderType::Vertex))
		{
			EAE6320_ASSERTF(false, "Can't initialize shading data without vertex shader");
		}
		if (!eae6320::Graphics::cShader::Load("data/Shaders/Fragment/MoleEnd.shader",
			fragmentShader, eae6320::Graphics::eShaderType::Fragment))
		{
			EAE6320_ASSERTF(false, "Can't initialize shading data without fragment shader");
		}
		eae6320::Graphics::cEffect::Load(endEffect, vertexShader, fragmentShader);
	}
	eae6320::Graphics::cMesh* mesh;
	{
		eae6320::Graphics::cMesh::Load(mesh, "data/Meshes/Mole.mesh");
	}
	for (int i = 0; i < 9; i++)
	{
		Graphics::cEffect** ppEffect = new Graphics::cEffect * [3];
		ppEffect[0] = inactiveEffect;
		ppEffect[1] = activeEffect;
		ppEffect[2] = endEffect;
		Graphics::cMesh** ppMesh = new Graphics::cMesh * [1];
		ppMesh[0] = mesh;
		if (!eae6320::GameObjects::cRenderableObject::Load(m_renderableObjects[i], gameObjectRigidBodyState, ppEffect, 3, ppMesh, 1))
		{
			EAE6320_ASSERTF(false, "Can't initialize renderableObject");
		}
		if(areas[i]) m_renderableObjects[i]->SetEffectIndex(1);
		else m_renderableObjects[i]->SetEffectIndex(0);
		int row = i / 3;
		int column = i % 3;
		m_renderableObjects[i]->SetPosition(Math::sVector(static_cast<float>(column) - 1.5f, static_cast<float>(row) - 1.5f, 0.0f));
	}
}

void eae6320::cMyGame::CleanUpGameObjects()
{
	for (int i = 0; i < 9; i++)
	{
		if (m_renderableObjects[i] != nullptr)
		{
			m_renderableObjects[i]->DecrementReferenceCount();
			m_renderableObjects[i] = nullptr;
		}
	}
}

int eae6320::cMyGame::ConvertToArea(int i_x, int i_y)
{
	int l = 0;
	int r = 0;
	if (i_x > 0 && i_x < 512 / 3) l = 0;
	else if (i_x > 512 / 3 && i_x < (512 * 2) / 3) l = 1;
	else if (i_x > (512 * 2) / 3 && i_x < 512) l = 2;
	else return -1;

	if (i_y > 0 && i_y < 512 / 3) r = 2;
	else if (i_y > 512 / 3 && i_y < (512 * 2) / 3) r = 1;
	else if (i_y > (512 * 2) / 3 && i_y < 512) r = 0;
	else return -1;

	return r * 3 + l;
}

int WINAPI WinMain(HINSTANCE i_thisInstanceOfTheApplication, HINSTANCE, char* i_commandLineArguments, int i_initialWindowDisplayState)
{
	return eae6320::Application::Run<eae6320::cMyGame>(i_thisInstanceOfTheApplication, i_commandLineArguments, i_initialWindowDisplayState);
}