/*
	This vertex shader is used to create a Direct3D vertex input layout object
*/

#include <Shaders/shaders.inc>

VERTEX_MAIN
{
#if defined( EAE6320_PLATFORM_D3D )
	// The shader program is only used to generate a vertex input layout object;
	// the actual shading code is never used
	o_vertexPosition_projected = VECTOR4(i_vertexPosition_local, 1.0);
#elif defined( EAE6320_PLATFORM_GL )
	// The shader program is only used by Direct3D
	gl_Position = VECTOR4(i_vertexPosition_local, 1.0);
#endif
}