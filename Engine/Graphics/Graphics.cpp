// Includes
//=========

#include "Graphics.h"

#include "cConstantBuffer.h"
#include "ConstantBufferFormats.h"
#include "cShader.h"
#include "cMesh.h"
#include "cEffect.h"
#include "cRenderTarget.h"
#include "sContext.h"
#include "VertexFormats.h"

#include <Engine/Asserts/Asserts.h>
#include <Engine/Concurrency/cEvent.h>
#include <Engine/Logging/Logging.h>
#include <Engine/Platform/Platform.h>
#include <Engine/ScopeGuard/cScopeGuard.h>
#include <Engine/Time/Time.h>
#include <Engine/UserOutput/UserOutput.h>
#include <utility>

// Static Data
//============
namespace 
{
	// Constant buffer object
	eae6320::Graphics::cConstantBuffer s_constantBuffer_frame(eae6320::Graphics::ConstantBufferTypes::Frame);
	eae6320::Graphics::cConstantBuffer s_constantBuffer_drawCall(eae6320::Graphics::ConstantBufferTypes::DrawCall);

	// Submission Data
	//----------------

	// This struct's data is populated at submission time;
	// it must cache whatever is necessary in order to render a frame
	struct sDataRequiredToRenderAFrame
	{
		eae6320::Graphics::ConstantBufferFormats::sFrame constantData_frame;
		float clearColor[4];
		int effectsDrawCallsAndMeshesCount = 0;
		eae6320::Graphics::sEffectDrawCallAndMesh* effectsDrawCallsAndMeshes = nullptr;
	};
	// In our class there will be two copies of the data required to render a frame:
	//	* One of them will be in the process of being populated by the data currently being submitted by the application loop thread
	//	* One of them will be fully populated and in the process of being rendered from in the render thread
	// (In other words, one is being produced while the other is being consumed)
	sDataRequiredToRenderAFrame s_dataRequiredToRenderAFrame[2];
	auto* s_dataBeingSubmittedByApplicationThread = &s_dataRequiredToRenderAFrame[0];
	auto* s_dataBeingRenderedByRenderThread = &s_dataRequiredToRenderAFrame[1];
	// The following two events work together to make sure that
	// the main/render thread and the application loop thread can work in parallel but stay in sync:
	// This event is signaled by the application loop thread when it has finished submitting render data for a frame
	// (the main/render thread waits for the signal)
	eae6320::Concurrency::cEvent s_whenAllDataHasBeenSubmittedFromApplicationThread;
	// This event is signaled by the main/render thread when it has swapped render data pointers.
	// This means that the renderer is now working with all the submitted data it needs to render the next frame,
	// and the application loop thread can start submitting data for the following frame
	// (the application loop thread waits for the signal)
	eae6320::Concurrency::cEvent s_whenDataForANewFrameCanBeSubmittedFromApplicationThread;

	// Geometry Data
	//--------------

	// Shading Data
	//-------------

	// RenderTarget
	//-------------
	eae6320::Graphics::cRenderTarget* s_renderTarget = nullptr;
}

// Helper Declarations
//====================

namespace
{
	/*
	eae6320::cResult InitializeGeometry();
	eae6320::cResult InitializeShadingData()
	*/
}

// Interface
//==========

// Submission
//-----------

void eae6320::Graphics::SubmitElapsedTime(const float i_elapsedSecondCount_systemTime, const float i_elapsedSecondCount_simulationTime)
{
	EAE6320_ASSERT(s_dataBeingSubmittedByApplicationThread);
	auto& constantData_frame = s_dataBeingSubmittedByApplicationThread->constantData_frame;
	constantData_frame.g_elapsedSecondCount_systemTime = i_elapsedSecondCount_systemTime;
	constantData_frame.g_elapsedSecondCount_simulationTime = i_elapsedSecondCount_simulationTime;
}

void eae6320::Graphics::SubmitClearColor(const float* i_clearColor)
{
	EAE6320_ASSERT(s_dataBeingSubmittedByApplicationThread);
	auto& clearColor = s_dataBeingSubmittedByApplicationThread->clearColor;
	memcpy(clearColor, i_clearColor, 4 * sizeof(float));
}

void eae6320::Graphics::SubmitEffectsDrawCallsAndMeshes(sEffectDrawCallAndMesh* i_effectsDrawCallsAndMeshes, const int i_effectsDrawCallsAndMeshesCount)
{
	EAE6320_ASSERT(s_dataBeingSubmittedByApplicationThread);
	s_dataBeingSubmittedByApplicationThread->effectsDrawCallsAndMeshesCount = i_effectsDrawCallsAndMeshesCount;
	s_dataBeingSubmittedByApplicationThread->effectsDrawCallsAndMeshes = i_effectsDrawCallsAndMeshes;
}

void eae6320::Graphics::SubmitCamera(const Math::cMatrix_transformation i_transform_worldToCamera, const Math::cMatrix_transformation i_transform_cameraToProjected)
{
	EAE6320_ASSERT(s_dataBeingSubmittedByApplicationThread);
	auto& constantData_frame = s_dataBeingSubmittedByApplicationThread->constantData_frame;
	constantData_frame.g_transform_worldToCamera = i_transform_worldToCamera;
	constantData_frame.g_transform_cameraToProjected = i_transform_cameraToProjected;
}


eae6320::cResult eae6320::Graphics::WaitUntilDataForANewFrameCanBeSubmitted(const unsigned int i_timeToWait_inMilliseconds)
{
	return Concurrency::WaitForEvent(s_whenDataForANewFrameCanBeSubmittedFromApplicationThread, i_timeToWait_inMilliseconds);
}

eae6320::cResult eae6320::Graphics::SignalThatAllDataForAFrameHasBeenSubmitted()
{
	return s_whenAllDataHasBeenSubmittedFromApplicationThread.Signal();
}

// Render
//-------

void eae6320::Graphics::RenderFrame()
{
	// Wait for the application loop to submit data to be rendered
	{
		if (Concurrency::WaitForEvent(s_whenAllDataHasBeenSubmittedFromApplicationThread))
		{
			// Switch the render data pointers so that
			// the data that the application just submitted becomes the data that will now be rendered
			std::swap(s_dataBeingSubmittedByApplicationThread, s_dataBeingRenderedByRenderThread);
			// Once the pointers have been swapped the application loop can submit new data
			if (!s_whenDataForANewFrameCanBeSubmittedFromApplicationThread.Signal())
			{
				EAE6320_ASSERTF(false, "Couldn't signal that new graphics data can be submitted");
				Logging::OutputError("Failed to signal that new render data can be submitted");
				UserOutput::Print("The renderer failed to signal to the application that new graphics data can be submitted."
					" The application is probably in a bad state and should be exited");
				return;
			}
		}
		else
		{
			EAE6320_ASSERTF(false, "Waiting for the graphics data to be submitted failed");
			Logging::OutputError("Waiting for the application loop to submit data to be rendered failed");
			UserOutput::Print("The renderer failed to wait for the application to submit data to be rendered."
				" The application is probably in a bad state and should be exited");
			return;
		}
	}

	auto& clearColor = s_dataBeingRenderedByRenderThread->clearColor;

	s_renderTarget->ClearBuffer(clearColor);

	EAE6320_ASSERT(s_dataBeingRenderedByRenderThread);

	// Update the frame constant buffer
	{
		// Copy the data from the system memory that the application owns to GPU memory
		auto& constantData_frame = s_dataBeingRenderedByRenderThread->constantData_frame;
		s_constantBuffer_frame.Update(&constantData_frame);
	}


	for (int i = 0; i < s_dataBeingRenderedByRenderThread->effectsDrawCallsAndMeshesCount; i++)
	{
		// Bind the shading data
		EAE6320_ASSERT(s_dataBeingRenderedByRenderThread->effectsDrawCallsAndMeshes[i].m_effect != nullptr);
		s_dataBeingRenderedByRenderThread->effectsDrawCallsAndMeshes[i].m_effect->Bind();
		//Update the draw call constant buffer
		auto& constantData_drawCall = s_dataBeingRenderedByRenderThread->effectsDrawCallsAndMeshes[i].m_constantData_drawCall;
		s_constantBuffer_drawCall.Update(&constantData_drawCall);
		// Draw the geometry
		EAE6320_ASSERT(s_dataBeingRenderedByRenderThread->effectsDrawCallsAndMeshes[i].m_mesh != nullptr);
		s_dataBeingRenderedByRenderThread->effectsDrawCallsAndMeshes[i].m_mesh->DrawGeometry();
	}

	s_renderTarget->SwapBuffer();

	// After all of the data that was submitted for this frame has been used
	// you must make sure that it is all cleaned up and cleared out
	// so that the struct can be re-used (i.e. so that data for a new frame can be submitted to it)
	{
		// (At this point in the class there isn't anything that needs to be cleaned up)
		//dataRequiredToRenderFrame	// TODO
		if (s_dataBeingRenderedByRenderThread->effectsDrawCallsAndMeshes)
		{
			for (int i = 0; i < s_dataBeingRenderedByRenderThread->effectsDrawCallsAndMeshesCount; i++)
			{
				if (s_dataBeingRenderedByRenderThread->effectsDrawCallsAndMeshes[i].m_effect != nullptr)
				{
					s_dataBeingRenderedByRenderThread->effectsDrawCallsAndMeshes[i].m_effect->DecrementReferenceCount();
					s_dataBeingRenderedByRenderThread->effectsDrawCallsAndMeshes[i].m_effect = nullptr;
				}
				if (s_dataBeingRenderedByRenderThread->effectsDrawCallsAndMeshes[i].m_mesh != nullptr)
				{
					s_dataBeingRenderedByRenderThread->effectsDrawCallsAndMeshes[i].m_mesh->DecrementReferenceCount();
					s_dataBeingRenderedByRenderThread->effectsDrawCallsAndMeshes[i].m_mesh = nullptr;
				}
			}
			delete[] s_dataBeingRenderedByRenderThread->effectsDrawCallsAndMeshes;
		}
	}
}

// Initialize / Clean Up
//--------------------------

eae6320::cResult eae6320::Graphics::Initialize(const sInitializationParameters& i_initializationParameters)
{
	auto result = Results::Success;

	// Initialize the platform-specific context
	if (!(result = sContext::g_context.Initialize(i_initializationParameters)))
	{
		EAE6320_ASSERTF(false, "Can't initialize Graphics without context");
		return result;
	}
	// Initialize the platform-independent graphics objects
	{
		if (result = s_constantBuffer_frame.Initialize())
		{
			// There is only a single frame constant buffer that is reused
			// and so it can be bound at initialization time and never unbound
			s_constantBuffer_frame.Bind(
				// In our class both vertex and fragment shaders use per-frame constant data
				static_cast<uint_fast8_t>(eShaderType::Vertex) | static_cast<uint_fast8_t>(eShaderType::Fragment));
		}
		else
		{
			EAE6320_ASSERTF(false, "Can't initialize Graphics without frame constant buffer");
			return result;
		}

		if (result = s_constantBuffer_drawCall.Initialize())
		{
			// There is only a single frame constant buffer that is reused
			// and so it can be bound at initialization time and never unbound
			s_constantBuffer_drawCall.Bind(
				// In our class both vertex and fragment shaders use per-frame constant data
				static_cast<uint_fast8_t>(eShaderType::Vertex) | static_cast<uint_fast8_t>(eShaderType::Fragment));
		}
		else
		{
			EAE6320_ASSERTF(false, "Can't initialize Graphics without drawCall constant buffer");
			return result;
		}
	}
	// Initialize the events
	{
		if (!(result = s_whenAllDataHasBeenSubmittedFromApplicationThread.Initialize(Concurrency::EventType::ResetAutomaticallyAfterBeingSignaled)))
		{
			EAE6320_ASSERTF(false, "Can't initialize Graphics without event for when data has been submitted from the application thread");
			return result;
		}
		if (!(result = s_whenDataForANewFrameCanBeSubmittedFromApplicationThread.Initialize(Concurrency::EventType::ResetAutomaticallyAfterBeingSignaled,
			Concurrency::EventState::Signaled)))
		{
			EAE6320_ASSERTF(false, "Can't initialize Graphics without event for when data can be submitted from the application thread");
			return result;
		}
	}

	// Initialize the render target
	{
		if (!(result = eae6320::Graphics::cRenderTarget::Load(s_renderTarget, i_initializationParameters)))
		{
			EAE6320_ASSERTF(false, "Can't initialize render target");
			return result;
		}
	}

	return result;
}

eae6320::cResult eae6320::Graphics::CleanUp()
{
	auto result = Results::Success;

	if (s_renderTarget)
	{
		s_renderTarget->DecrementReferenceCount();
		s_renderTarget = nullptr;
	}

	// data submitted for a frame that never gets rendered, clean up thems
	{
		if (s_dataBeingSubmittedByApplicationThread->effectsDrawCallsAndMeshes != nullptr)
		{
			for (int i = 0; i < s_dataBeingSubmittedByApplicationThread->effectsDrawCallsAndMeshesCount; i++)
			{
				if (s_dataBeingSubmittedByApplicationThread->effectsDrawCallsAndMeshes[i].m_effect != nullptr)
				{
					s_dataBeingSubmittedByApplicationThread->effectsDrawCallsAndMeshes[i].m_effect->DecrementReferenceCount();
					s_dataBeingSubmittedByApplicationThread->effectsDrawCallsAndMeshes[i].m_effect = nullptr;
				}
				if (s_dataBeingSubmittedByApplicationThread->effectsDrawCallsAndMeshes[i].m_mesh != nullptr)
				{				
					s_dataBeingSubmittedByApplicationThread->effectsDrawCallsAndMeshes[i].m_mesh->DecrementReferenceCount();
					s_dataBeingSubmittedByApplicationThread->effectsDrawCallsAndMeshes[i].m_mesh = nullptr;
				}
			}
			delete[] s_dataBeingSubmittedByApplicationThread->effectsDrawCallsAndMeshes;
		}
	}

	{
		const auto result_constantBuffer_frame = s_constantBuffer_frame.CleanUp();
		if (!result_constantBuffer_frame)
		{
			EAE6320_ASSERT(false);
			if (result)
			{
				result = result_constantBuffer_frame;
			}
		}
	}

	{
		const auto result_constantBuffer_drawCall = s_constantBuffer_drawCall.CleanUp();
		if (!result_constantBuffer_drawCall)
		{
			EAE6320_ASSERT(false);
			if (result)
			{
				result = result_constantBuffer_drawCall;
			}
		}
	}

	{
		const auto result_context = sContext::g_context.CleanUp();
		if (!result_context)
		{
			EAE6320_ASSERT(false);
			if (result)
			{
				result = result_context;
			}
		}
	}

	return result;
}

// Helper Definitions
//===================

namespace

{
	
}