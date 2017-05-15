//------------------------------------------------------------
// Domain shader section
//------------------------------------------------------------
struct DS_OUTPUT
{
	float4 vPosition	: SV_POSITION;
	float3 vWorldPos	: WORLDPOS;
	float3 vNormal		: NORMAL;
	float3 vBinormal	: BINORMAL;
	float3 vTangent		: TANGENT;
};

//------------------------------------------------------------
// Solid color shading pixel shader (used for wireframe
// overlay)
//------------------------------------------------------------
float4 psOceanWired(DS_OUTPUT Input) : SV_TARGET
{
	// Return a solid white color
	return float4(1, 1, 1, 1);
}
