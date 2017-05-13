#include "Input_PS.hlsli"

//--------------------------------------------------------------------------------------
// Pixel Shader (Normals)
//--------------------------------------------------------------------------------------
float4 psNormal(PS_INPUT input) : SV_Target
{
	return input.Norm;
}
