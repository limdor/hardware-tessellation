//Background
TextureCube envMap : register(t0);

SamplerState linearSampler : register(s0);

struct QuadOutput
{
	float4 pos		: SV_POSITION;
	float2 tex		: TEXCOORD0;
	float3 viewDir	: TEXCOORD1;
};

float4 psBackground(QuadOutput input) : SV_TARGET
{
	return envMap.Sample(linearSampler, input.viewDir);
};
