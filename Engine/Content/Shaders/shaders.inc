/*
	This file should be #included by all shaders
*/

// Version Information
//====================

#if defined( EAE6320_PLATFORM_GL )

	// GLSL shaders require the version to be #defined before anything else in the shader
	#version 420

#endif

// Data Type
//=================

#if defined( EAE6320_PLATFORM_D3D )
#define VECTOR4 float4
#define MATRIX4 float4x4
#define VECTOR2 float2
#define MULTIPLY(v1,v2) mul(v1,v2)
#elif defined( EAE6320_PLATFORM_GL )
#define VECTOR4 vec4
#define MATRIX4 mat4
#define VECTOR2 vec2
#define MULTIPLY(v1,v2) v1*v2
#endif


// Constant Buffers
//=================

#if defined( EAE6320_PLATFORM_D3D )
#define DeclareConstantBuffer( i_name, i_id ) \
	cbuffer i_name : register( b##i_id )
#elif defined( EAE6320_PLATFORM_GL )
#define DeclareConstantBuffer( i_name, i_id ) \
	layout( std140, binding = i_id ) uniform i_name
#endif

#if defined( EAE6320_PLATFORM_D3D )
#define FRAGMENT_MAIN \
	void main(\
	in const float4 i_fragmentPosition : SV_POSITION,\
	out float4 o_color : SV_TARGET\
)
#elif defined( EAE6320_PLATFORM_GL )
#define FRAGMENT_MAIN \
	out vec4 o_color;\
	void main()
#endif

#if defined( EAE6320_PLATFORM_D3D )
#define VERTEX_MAIN \
	void main(\
	in const float3 i_vertexPosition_local : POSITION,\
	out float4 o_vertexPosition_projected : SV_POSITION\
)
#elif defined( EAE6320_PLATFORM_GL )
#define VERTEX_MAIN \
	layout( location = 0 ) in vec3 i_vertexPosition_local;\
	void main()
#endif

DeclareConstantBuffer(g_constantBuffer_frame, 0)
{
	MATRIX4 g_transform_worldToCamera;
	MATRIX4 g_transform_cameraToProjected;
	float g_elapsedSecondCount_systemTime;
	float g_elapsedSecondCount_simulationTime;
	VECTOR2 g_padding;
};

DeclareConstantBuffer(g_constantBuffer_drawCall, 2)
{
	MATRIX4 g_transform_localToWorld;
};