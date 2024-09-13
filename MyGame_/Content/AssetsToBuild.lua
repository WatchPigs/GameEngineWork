--[[
	This file lists every asset that must be built by the AssetBuildSystem
]]

return
{
	shaders =
	{
		{ path = "Shaders/Vertex/standard.shader", arguments = { "vertex" } },
		{ path = "Shaders/Fragment/MoleActive.shader", arguments = { "fragment" } },
		{ path = "Shaders/Fragment/MoleInactive.shader", arguments = { "fragment" } },
		{ path = "Shaders/Fragment/MoleEnd.shader", arguments = { "fragment" } },
		{ path = "Shaders/Vertex/vertexInputLayout_mesh.shader", arguments = { "vertex" } },
	},
	meshes = 
	{
		{ path = "Meshes/Mole.mesh" },
	}
}
