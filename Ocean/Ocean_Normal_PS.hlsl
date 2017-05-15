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
// Normal color shading pixel shader (used for normal overlay)
//------------------------------------------------------------
float4 psOceanNormal(DS_OUTPUT input) : SV_TARGET
{
	//We have to normalize again because can be not normalized
	float3 N = normalize(input.vNormal);
	return float4(N, 1);
}
