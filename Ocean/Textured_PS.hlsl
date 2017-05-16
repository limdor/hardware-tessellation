#include "Input_PS.hlsli"

//--------------------------------------------------------------------------------------
// Constant Buffer Variables
//--------------------------------------------------------------------------------------
Texture2D txDiffuse : register(t0);
SamplerState samLinear : register(s0);

cbuffer cbChangesEveryFrame : register(b2)
{
	matrix World;
	float4 vMeshColor;
};

//--------------------------------------------------------------------------------------
// Pixel Shader (Textured)
//--------------------------------------------------------------------------------------
float4 psTextured(PS_INPUT input) : SV_Target
{
	return txDiffuse.Sample(samLinear, input.Tex) * vMeshColor;
}
