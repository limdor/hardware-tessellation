//------------------------------------------------------------
// Constant Buffers
//------------------------------------------------------------
cbuffer transform : register(b0)
{
	float4x4 viewProjMatrix;
	float4x4 orientProjMatrixInverse;
	float3 eyePosition;
}

cbuffer cbChangesEveryFrame : register(b1)
{
	float time;
};

cbuffer configuration : register(b2)
{
	float minDistance;
	float maxDistance;
	float minTessExp;
	float maxTessExp;
	float sizeTerrain;
	bool applyCorrection;
}

Texture2D bumpTexture : register(t0);

//Background
TextureCube envMap : register(t1);

SamplerState linearSampler : register(s0);


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
// Shading pixel shader section
//------------------------------------------------------------

// The pixel shader works the same as it would in a normal
// graphics pipeline. In this case, we apply a Fresnel
// reflection and we add some noise to the normal vector.

float4 psOcean(DS_OUTPUT input) : SV_TARGET
{
	float3 positionWorld = input.vWorldPos;
	float2 coord = float2(positionWorld.x, positionWorld.z) / sizeTerrain;

	//We add some noise to get a more realistic water
	float4 t0 = bumpTexture.Sample(linearSampler, float2(coord * 100 * 0.02 + 2 * 0.07 * float2(1,0) * (time / 10) * float2(-1,-1)));
	float4 t1 = bumpTexture.Sample(linearSampler, float2(coord * 100 * 0.03 + 2 * 0.057 * float2(0.86, 0.5) * (time / 10) * float2(1,-1)));
	float4 t2 = bumpTexture.Sample(linearSampler, float2(coord * 100 * 0.05 + 2 * 0.047 * float2(0.86, -0.5) * (time / 10) * float2(-1,1)));
	float4 t3 = bumpTexture.Sample(linearSampler, float2(coord * 100 * 0.07 + 2 * 0.037 * float2(0.7, 0.7) * (time / 10)));

	float3 N = (t0 + t1 + t2 + t3).xzy * 2.0 - 3.3;
	N.xz *= 2.0;

	N = normalize(N);

	float3x3 m;
	m[0] = input.vTangent;
	m[1] = input.vNormal;
	m[2] = input.vBinormal;

	float3 normalWorld = mul(N, m);
	normalWorld = normalize(normalWorld);
	//normalWorld = input.vNormal; //remove bumps

	float3 toCamera = normalize(eyePosition - positionWorld);

	float cosAngle = max(dot(toCamera, normalWorld), 0);
	float F = 0.02 + 0.98 * pow((1 - cosAngle), 5);

	float4 shallowColor = float4 (0.56, 0.62, 0.8 ,0);
	float4 deepColor = float4(0.2, 0.3, 0.5, 0);

	float3 reflDir = reflect(eyePosition - positionWorld, N);
	float4 envColor = envMap.Sample(linearSampler, -reflDir);

	float4 waterColor = deepColor * cosAngle + shallowColor * (1 - cosAngle);

	float4 color = envColor * F + waterColor * (1 - F);

	return color;
}
