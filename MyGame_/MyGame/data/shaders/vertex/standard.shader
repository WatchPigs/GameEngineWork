DXBCфмтїУчђ8_ъ    [     8     <  p      RDEFШ     T          ўџA    <                             g_constantBuffer_frame Ћ<      l              ф       @               @   @             .            P      `            P                        g_transform_worldToCamera ЋЋ            g_transform_cameraToProjected g_elapsedSecondCount_systemTime ЋЋ             g_elapsedSecondCount_simulationTime g_padding ЋЋ            Microsoft (R) HLSL Shader Compiler 10.1 ISGN,                              POSITION ЋЋЋOSGN,                              SV_POSITION SHDR$  @     Y  F         _  r     g  ђ         h     6  r      F     6        @    ?6  ђ      F     8  ђ           F          8  ђ     V     F            ђ     F    F    8  ђ     І
     F            ђ     F    F    8  ђ      і     F            ђ      F     F    8  ђ           F         8  ђ     V     F            ђ     F    F    8  ђ     І
     F            ђ     F    F    8  ђ      і     F            ђ      F     F    >  STATt                                                                                                                 SPDB V  Microsoft C/C++ MSF 7.00
DS         +   д       )                                                                                                                                                                                                                                                                                                                                                                                                                                                                           Рџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџ8    ќџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџџ       <       џџџџ                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                         .1сH$c   ыРЌЌ
]Hгж|Рu                          мQ3                                                                                                                                                                                                                                                                                                                                                                                                                                                                    D3DSHDR $                             `                                                                                                                                                                                                                                                                                                                                                                                                                                                                                        Lш A$ ЦZ  Ѕ P 4 g 1ћ ЩО                                                                                                                                                                                                                                                                                                                                                                                                                                                                                             /*
	This is the standard vertex shader

	A vertex shader is responsible for two things:
		* Telling the GPU where the vertex (one of the three in a triangle) should be drawn on screen in a given window
			* The GPU will use this to decide which fragments (i.e. pixels) need to be shaded for a given triangle
		* Providing any data that a corresponding fragment shader will need
			* This data will be interpolated across the triangle and thus vary for each fragment of a triangle that gets shaded
*/

#include <Shaders/shaders.inc>

#if defined( EAE6320_PLATFORM_D3D )

// Constant Buffers
//=================

cbuffer g_constantBuffer_frame : register( b0 )
{
	float4x4 g_transform_worldToCamera;
	float4x4 g_transform_cameraToProjected;

	float g_elapsedSecondCount_systemTime;
	float g_elapsedSecondCount_simulationTime;
	// For float4 alignment
	float2 g_padding;
};

// Entry Point
//============

void main(

	// Input
	//======

	// The "semantics" (the keywords in all caps after the colon) are arbitrary,
	// but must match the C call to CreateInputLayout()

	// These values come from one of the VertexFormats::sVertex_mesh that the vertex buffer was filled with in C code
	in const float3 i_vertexPosition_local : POSITION,

	// Output
	//=======

	// An SV_POSITION value must always be output from every vertex shader
	// so that the GPU can figure out which fragments need to be shaded
	out float4 o_vertexPosition_projected : SV_POSITION

)
{
	// Transform the local vertex into world space
	float4 vertexPosition_world;
	{
		// This will be done in a future assignment.
		// For now, however, local space is treated as if it is the same as world space.
		float4 vertexPosition_local = float4( i_vertexPosition_local, 1.0 );
		vertexPosition_world = vertexPosition_local;
	}
	// Calculate the position of this vertex projected onto the display
	{
		// Transform the vertex from world space into camera space
		float4 vertexPosition_camera = mul( g_transform_worldToCamera, vertexPosition_world );
		// Project the vertex from camera space into projected space
		o_vertexPosition_projected = mul( g_transform_cameraToProjected, vertexPosition_camera );
	}
}

#elif defined( EAE6320_PLATFORM_GL )

// Constant Buffers
//=================

layout( std140, binding = 0 ) uniform g_constantBuffer_frame
{
	mat4 g_transform_worldToCamera;
	mat4 g_transform_cameraToProjected;

	float g_elapsedSecondCount_systemTime;
	float g_elapsedSecondCount_simulationTime;
	// For vec4 alignment
	vec2 g_padding;
};

// Input
//======

// The locations assigned are arbitrary
// but must match the C calls to glVertexAttribPointer()

// These values come from one of the VertexFormats::sVertex_mesh that the vertex buffer was filled with in C code
layout( location = 0 ) in vec3 i_vertexPosition_local;

// Output
//=======

// The vertex shader must always output a position value,
// but unlike HLSL where the value is explicit
// GLSL has an automatically-required variable named "gl_Position"

// Entry Point
//============

void main()
{
	// Transform the local vertex into world space
	vec4 vertexPosition_world;
	{
		// This will be done in a future assignment.
		// For now, however, local space is treated as if it is the same as world space.
		vec4 vertexPosition_local = vec4( i_vertexPosition_local, 1.0 );
		vertexPosition_world = vertexPosition_local;
	}
	// Calculate the position of this vertex projected onto the display
	{
		// Transform the vertex from world space into camera space
		vec4 vertexPosition_camera = g_transform_worldToCamera * vertexPosition_world;
		// Project the vertex from camera space into projected space
		gl_Position = g_transform_cameraToProjected * vertexPosition_camera;
	}
}

#endif
                                                                                                                                                                                                                                                      /*
	This file should be #included by all shaders
*/

// Version Information
//====================

#if defined( EAE6320_PLATFORM_GL )

	// GLSL shaders require the version to be #defined before anything else in the shader
	#version 420

#endif

// Constant Buffers
//=================

#if defined( EAE6320_PLATFORM_D3D )

#define DeclareConstantBuffer( i_name, i_id ) \
	cbuffer i_name : register( b##i_id )\
	{\
		float4x4 g_transform_worldToCamera;\
		float4x4 g_transform_cameraToProjected;\
		float g_elapsedSecondCount_systemTime;\
		float g_elapsedSecondCount_simulationTime;\
		float2 g_padding;\
	}

#elif defined( EAE6320_PLATFORM_GL )

#define DeclareConstantBuffer( i_name, i_id ) \
	layout( std140, binding = i_id ) uniform i_name\
	{\
		mat4 g_transform_worldToCamera;\
		mat4 g_transform_cameraToProjected;\
		float g_elapsedSecondCount_systemTime;\
		float g_elapsedSecondCount_simulationTime;\
		vec2 g_padding;\
	}

#endif
                                        ўяўя   Ќ   E:\C++ Projects\Eng2\zhouyexiang\Engine\Content\shaders\vertex\standard.shader  e:\c++ projects\eng2\zhouyexiang\engine\content\shaders\vertex\standard.shader Shaders/shaders.inc shaders\shaders.inc /*
	This is the standard vertex shader

	A vertex shader is responsible for two things:
		* Telling the GPU where the vertex (one of the three in a triangle) should be drawn on screen in a given window
			* The GPU will use this to decide which fragments (i.e. pixels) need to be shaded for a giт0Ќ   =№пяВЩи                                                               Д   (   т0ыпAМи      P   Д               Q   (   т01,О
     P   Q                                                                                                                                                                                                                                                                                                                                                                      B <   
  daJ
  daJMicrosoft (R) HLSL Shader Compiler 10.1   ^ =hlslFlags 0x44005 hlslTarget vs_4_0 hlslEntry main hlslDefines  /DEAE6320_PLATFORM_D3D     *     и      ш      ш    <     main > >  	 i_vertexPosition_local                                 P     <    ш     P    <    ш    P    <    ш   B >  	 o_vertexPosition_projected                                 P     <    ш     P    <    ш    P    <    ш    P    <    ш   : >   vertexPosition_world                               P      x    И      P     x    И     P     x    И     P     x    И    : >   vertexPosition_local                               P      P    р      P     P    р     P     P    р     P     d    Ь    > >   vertexPosition_camera                                  P      L   И      P     L   И     P     L   И     P     L   И      є   0      Яд^0N%р%\ё      mжR) }юЃ&ёФи  ђ   Ш        $      $   М  <   9  <   9   P   9  P   9   d   :  d   :   x   ?  x   ?      ?     ?   И   ?  И   ?   д   ?  д   ?   є   ?  є   ?     ?    ?   0  ?  0  ?   L  A  L  A   l  A  l  A     A    A   Ј  A  Ј  A   Ш  A  Ш  A   ф  A  ф  A     A    A      C     C    F 
 E  F 
 E  .  -  X " W  X " W  X " W  X " W  X " W  X " W  X " W  [   Z  [   Z  [   Z  [   Z  [   Z  [   Z  [   Z     і                    D   Ъ18      	       џџ   џџ     $   $      ,        @       float3 ѓђё
       @       float4 ѓђё        
              @             @ float4x4 
     
                                                                                                                                                                                                                                                                                                             Ъ18              џџ   џџ                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                 џџџџ	/ё     E                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                       %    Ј    main  . Q       џџџџџџg_transform_worldToCamera   2 Q     @ џџџџџџg_transform_cameraToProjected                                                                                                                                                                                                                                                                                                                                                                                                                                      џџџџ	/ё                                                                                                                                                                                                                                                                                                                                                                                                                                                                                            ven triangle
		* Providing any data that a corresponding fragment shader will need
			* This data will be interpolated across the triangle and thus vary for each fragment of a triangle that gets shaded
*/

#include <Shaders/shaders.inc>

#if defined( EAE6320_PLATFORM_D3D )

// Constant Buffers
//=================

cbuffer g_constantBuffer_frame : register( b0 )
{
	float4x4 g_transform_worldToCamera;
	float4x4 g_transform_cameraToProjected;

	float g_elapsedSecondCount_systemTime;
	float g_џџџџw	1      ?\   L       ,   t                                    $     `             
 м         Ѕ            main none   -К.ё       $     `                    џџџџ    $        џџџџ    џџџџ         O   E:\C++ Projects\Eng2\zhouyexiang\Engine\Content\shaders\vertex\standard.shader Shaders/shaders.inc  ўяўя                  џџџџџџџџџџ џџџџџџџџџџ                                                                                                                                     elapsedSecondCount_simulationTime;
	// For float4 alignment
	float2 g_padding;
};

// Entry Point
//============

void main(

	// Input
	//======

	// The "semantics" (the keywords in all caps after the colon) are arbitrary,
	// but must match the C call to CreateInputLayout()

	// These values come from one of the VertexFormats::sVertex_mesh that the vertex buffer was filled with in C code
	in const float3 i_vertexPosition_local : POSITION,

	// Output
	//=======

	// An SV_POSITION value must always be output from every vertex shader
	// so that the GPU can figure out which fragments need to be shaded
	out float4 o_vertexPosition_projected : SV_POSITION

)
{
	// Transform the local vertex into world space
	float4 vertexPosition_world;
	{
		// This will be done in a future assignment.
		// For now, however, local space is treated as if it is the same as world space.
		float4 vertexPosition_local = float4( i_vertexPosition_local, 1.0 );
		vertexPosition_world = vertexPosition_local;
	}
	// Calculate the position of this vertex projected onto the display
	{
		// Transform the vertex from world space into camera space
		float4 vertexPosition_camera = mul( g_transform_worldToCamera, vertexPosition_world );
		// Project the vertex from camera space into projected space
		o_vertexPosition_projected = mul( g_transform_cameraToProjected, vertexPosition_camera );
	}
}

#elif defined( EAE6320_PLATFORM_GL )

// Constant Buffers
//=================

layout( std140, binding = 0 ) uniform g_constantBuffer_frame
{
	mat4 g_transform_worldToCamera;
	mat4 g_transform_cameraToProjected;

	float g_elapsedSecondCount_systemTime;
	float g_elapsedSecondCount_simulationTime;
	// For vec4 alignment
	vec2 g_padding;
};

// Input
//======

// The locations assigned are arbitrary
// but must match the C calls to glVertexAttribPointer()

// These values come from one of the VertexFormats::sVertex_mesh that the vertex buffer was filled with in C code
layout( location = 0 ) in vec3 i_vertexPosition_local;

// Output
//=======

// The vertex shader must always output a position value,
// but unlike HLSL where the value is explicit
// GLSL has an automatically-required variable named "gl_Position"

// Entry Point
//============

void main()
{
	// Transform the local vertex into world space
	vec4 vertexPosition_world;
	{
		// This will be done in a future assignment.
		// For now, however, local space is treated as if it is the same as world space.
		vec4 vertexPosition_local = vec4( i_vertexPosition_local, 1.0 );
		vertexPosition_world = vertexPosition_local;
	}
	// Calculate the position of this vertex projected onto the display
	{
		// Transform the vertex from world space into camera space
		vec4 vertexPosition_camera = g_transform_worldToCamera * vertexPosition_world;
		// Project the vertex from camera space into projected space
		gl_Position = g_transform_cameraToProjected * vertexPosition_camera;
	}
}

#endif
 /*
	This file should be #included by all shaders
*/

// Version Information
//====================

#if defined( EAE6320_PLATFORM_GL )

	// GLSL shaders require the version to be #defined before anything else in the shader
	#version 420

#endif

// Constant Buffers
//=================

#if defined( EAE6320_PLATFORM_D3D )

#define DeclareConstantBuffer( i_name, i_id ) \
	cbuffer i_name : register( b##i_id )\
	{\
		float4x4 g_transform_worldToCamera;\
		float4x4 g_transform_cameraToProjected;\
		float g_elapsedSecondCount_systemTime;\
		float g_elapsedSecondCount_simulationTime;\
		float2 g_padding;\
	}

#elif defined( EAE6320_PLATFORM_GL )

#define DeclareConstantBuffer( i_name, i_id ) \
	layout( std140, binding = i_id ) uniform i_name\
	{\
		mat4 g_transform_worldToCamera;\
		mat4 g_transform_cameraToProjected;\
		float g_elapsedSecondCount_systemTime;\
		float g_elapsedSecondCount_simulationTime;\
		vec2 g_padding;\
	}

#endif
        P   г  Д          Q   Ш                                      .1сH$c   ыРЌЌ
]Hгж|Рu   /LinkInfo /names /src/headerblock /src/files/e:\c++ projects\eng2\zhouyexiang\engine\content\shaders\vertex\standard.shader /src/files/shaders\shaders.inc    
      >       |   	                "      
          мQ3                                                                                                                                                                                                                                                                        џ   и   {  8       ь  Ќ   
  и     ,       (   8  ,   x      '                         !   "   #   $   %   &         	   
                                                                                                                                                                                                                                                                                                                                                               (                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                               