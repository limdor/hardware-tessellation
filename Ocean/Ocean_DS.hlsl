// The output patch size. It is also 4 control points.
#define OUTPUT_PATCH_SIZE 4

//wave record
struct Wave
{
	float		wavelength;
	float		amplitude;
	float3		direction;
};

//wave data
#define NWAVES 8
static const Wave wave[NWAVES] = { { 65.0f, 1.0f, float3(0.98, 0, 0.17) },
{ 43.5f, 0.6f, float3(0.98, 0, -0.17) },
{ 22.0f, 0.4f, float3(0.934, 0, 0.342) },
{ 99.0f, 2.0f, float3(0.934, 0, -0.342) },
{ 25.0f, 0.3f, float3(0.97, 0, 0.24) },
{ 47.5f, 0.5f, float3(0.97, 0, -0.24) },
{ 57.0f, 0.9f, float3(0.99, 0, -0.14) },
{ 81.0f, 1.6f, float3(0.99, 0, 0.14) }
};

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


//------------------------------------------------------------
// Constant data function for the hsOcean. This is executed
// once per patch.
//------------------------------------------------------------
struct HS_CONSTANT_DATA_OUTPUT
{
	float Edges[4]	: SV_TessFactor;
	float Inside[2]	: SV_InsideTessFactor;
};

struct VERTEX_POSITION
{
	float3 vPosition	: POSITION;
};


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

// The domain shader is run once per vertex and calculates the
// final vertex's position and attributes. It receives the UVW
// from the fixed function tessellator and the control point
// outputs from the hull shader. Since we are using the
// DirectX 11 Tessellation pipeline, it is the domain shader's
// responsibility to calculate the final SV_POSITION for each
// vertex.
// The input SV_DomainLocation to the domain shader comes from
// fixed function tessellator. And the OutputPatch comes from
// the hull shader. From these, we must calculate the final
// vertex position, color, texcoords, and other attributes.
// The output from the domain shader will be a vertex that
// will go to the video card's rasterization pipeline and get
// drawn to the screen.

[domain("quad")]
DS_OUTPUT dsOcean(HS_CONSTANT_DATA_OUTPUT input, float2 UV : SV_DomainLocation, const OutputPatch<VERTEX_POSITION, OUTPUT_PATCH_SIZE> patch)
{
	//Linear interpolation between the position of the corners
	float WorldPosX = lerp(patch[0].vPosition.x, patch[2].vPosition.x, UV.x);
	float WorldPosZ = lerp(patch[0].vPosition.z, patch[2].vPosition.z, UV.y);
	
	//Calculating the position and normal of every vertex
	//depending on the waves
	float3 du = float3(1, 0, 0);
	float3 dv = float3(0, 0, 1);
	float3 displacement = 0;

	//For every wave
	for (int i = 0; i<NWAVES; i++)
	{
		float pdotk = dot(float3(WorldPosX, 0, WorldPosZ), wave[i].direction);
		float vel = sqrt(1.5621f * wave[i].wavelength);
		float phase = 6.28f / wave[i].wavelength *(pdotk + vel * time);

		float2 offset;
		sincos(phase, offset.x, offset.y);
		offset.x *= -wave[i].amplitude;
		offset.y *= -wave[i].amplitude;

		//We acumulate the displacement of every wave
		displacement += float3(wave[i].direction.x * offset.x, -offset.y, wave[i].direction.z * offset.x);

		float3 da = float3(wave[i].direction.x * offset.y, offset.x, wave[i].direction.z * offset.y);
		da *= 6.28 / wave[i].wavelength;
		du += da * wave[i].direction.x;
		dv += da * wave[i].direction.z;

	}
	
	//Calculation of the final position
	float3 position = float3(WorldPosX, 0, WorldPosZ) + displacement;

	DS_OUTPUT Output;

	Output.vPosition = mul(float4(position, 1), viewProjMatrix);
	Output.vWorldPos = position;
	Output.vBinormal = normalize(du);
	Output.vTangent = normalize(dv);
	Output.vNormal = normalize(cross(dv, du));

	return Output;
}
