//------------------------------------------------------------
// Constant Buffers
//------------------------------------------------------------
cbuffer transform : register(b0)
{
	float4x4 viewProjMatrix;
	float4x4 orientProjMatrixInverse;
	float3 eyePosition;
}

//This last part of the fx file is to render the background
struct QuadInput
{
	float3  pos	: POSITION;
	float2  tex	: TEXCOORD0;
};

struct QuadOutput
{
	float4 pos		: SV_POSITION;
	float2 tex		: TEXCOORD0;
	float3 viewDir	: TEXCOORD1;
};

QuadOutput vsQuad(QuadInput input)
{
	QuadOutput output = (QuadOutput)0;

	output.pos = float4(input.pos, 1.0f);
	float4 hWorldPosMinusEye = mul(float4(input.pos, 1.0f), orientProjMatrixInverse);
	hWorldPosMinusEye /= hWorldPosMinusEye.w;
	output.viewDir = hWorldPosMinusEye.xyz;
	output.pos.z = 0.99999;
	output.tex = input.tex;
	return output;
};
