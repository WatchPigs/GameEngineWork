/*
	This is the standard fragment shader

	A fragment shader is responsible for telling the GPU what color a specific fragment should be
*/

#include <Shaders/shaders.inc>

FRAGMENT_MAIN
{
	o_color = VECTOR4(
		// RGB (color)
		1.0, 1.0, 1.0,
		// Alpha (opacity)
		1.0);
	o_color.r = sin(4 * g_elapsedSecondCount_simulationTime) > 0.0f ? sin(4 * g_elapsedSecondCount_simulationTime) : 0.0f;
	o_color.g = 0.0f;
	o_color.b = -sin(4 * g_elapsedSecondCount_simulationTime) > 0.0f ? -sin(4 * g_elapsedSecondCount_simulationTime) : 0.0f;
}