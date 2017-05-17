// This allows us to compile the shader with a #define to
// choose the different partition modes for the hull shader.
// See the hull shader: [partitioning(OCEAN_HS_PARTITION)]
#ifndef OCEAN_HS_PARTITION
#define OCEAN_HS_PARTITION "fractional_even"
#endif

// The input patch size. It is 4 control points, the number we
// use to define a patch. This value should match the call to
// IASetPrimitiveTopology()
#define INPUT_PATCH_SIZE 4

// The output patch size. It is also 4 control points.
#define OUTPUT_PATCH_SIZE 4

//------------------------------------------------------------
// Constant Buffers
//------------------------------------------------------------
cbuffer transform : register(b0)
{
	float4x4 viewProjMatrix;
	float4x4 orientProjMatrixInverse;
	float3 eyePosition;
}

cbuffer configuration : register(b1)
{
	float minDistance;
	float maxDistance;
	float minTessExp;
	float maxTessExp;
	float sizeTerrain;
	bool applyCorrection;
}

//------------------------------------------------------------
// Vertex shader section
//------------------------------------------------------------
struct VERTEX_POSITION
{
	float3 vPosition	: POSITION;
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

// This constant hull shader is executed once per patch. It
// will be executed SQRT_NUMBER_OF_PATCHS *
// SQRT_NUMBER_OF_PATCHS times. We calculate a variable
// tessellation factor based on the angle and the distnace
// of the camera.

HS_CONSTANT_DATA_OUTPUT OceanConstantHS(InputPatch<VERTEX_POSITION, INPUT_PATCH_SIZE> ip, uint PatchID : SV_PrimitiveID)
{
	HS_CONSTANT_DATA_OUTPUT Output;

	//Calculating the central point of the patch and the central
	//point for every edge
	float3 middlePoint = (ip[0].vPosition + ip[1].vPosition + ip[2].vPosition + ip[3].vPosition) / 4;
	float3 middlePointEdge0 = (ip[0].vPosition + ip[1].vPosition) / 2;
	float3 middlePointEdge1 = (ip[3].vPosition + ip[0].vPosition) / 2;
	float3 middlePointEdge2 = (ip[2].vPosition + ip[3].vPosition) / 2;
	float3 middlePointEdge3 = (ip[1].vPosition + ip[2].vPosition) / 2;

	//Calculating a correction that will be applied to
	//tessellation factor depending on the angle of the camera
	//We calculate a correction for every central point of the
	//edges and the middle to avoid some lines between patches.
	float2 correctionInside = float2(1, 1);
	float4 correctionEdges = float4(1, 1, 1, 1);
	if (applyCorrection)
	{
		float3 insideDirection0 = normalize(middlePointEdge0 - middlePointEdge2);
		float3 insideDirection1 = normalize(middlePointEdge1 - middlePointEdge3);
		float3 edgeDirection0 = normalize(ip[0].vPosition - ip[1].vPosition);
		float3 edgeDirection1 = normalize(ip[3].vPosition - ip[0].vPosition);
		float3 edgeDirection2 = normalize(ip[2].vPosition - ip[3].vPosition);
		float3 edgeDirection3 = normalize(ip[1].vPosition - ip[2].vPosition);

		float3 toCameraMiddle = normalize(eyePosition - middlePoint);
		float3 toCameraEdge0 = normalize(eyePosition - middlePointEdge0);
		float3 toCameraEdge1 = normalize(eyePosition - middlePointEdge1);
		float3 toCameraEdge2 = normalize(eyePosition - middlePointEdge2);
		float3 toCameraEdge3 = normalize(eyePosition - middlePointEdge3);

		//Rank is used to decide how important is the angle correction in front of the distance
		float rank = 0.4;
		float shift = 1 - rank / 2;
		correctionInside.x = ((1.57 - acos(abs(dot(toCameraMiddle, insideDirection0)))) / 1.57) * rank + shift;
		correctionInside.y = ((1.57 - acos(abs(dot(toCameraMiddle, insideDirection1)))) / 1.57) * rank + shift;

		correctionEdges.x = ((1.57 - acos(abs(dot(toCameraEdge0, edgeDirection0)))) / 1.57) * rank + shift;
		correctionEdges.y = ((1.57 - acos(abs(dot(toCameraEdge1, edgeDirection1)))) / 1.57) * rank + shift;
		correctionEdges.z = ((1.57 - acos(abs(dot(toCameraEdge2, edgeDirection2)))) / 1.57) * rank + shift;
		correctionEdges.w = ((1.57 - acos(abs(dot(toCameraEdge3, edgeDirection3)))) / 1.57) * rank + shift;
	}

	//Calculating the distance from the central point of the
	//patch to the camera and from every central point of every
	//edge to the camera
	float magnitude = clamp(distance(middlePoint, eyePosition), minDistance, maxDistance);
	float4 magnitudeEdges;
	magnitudeEdges.x = clamp(distance(middlePointEdge0, eyePosition), minDistance, maxDistance);
	magnitudeEdges.y = clamp(distance(middlePointEdge1, eyePosition), minDistance, maxDistance);
	magnitudeEdges.z = clamp(distance(middlePointEdge2, eyePosition), minDistance, maxDistance);
	magnitudeEdges.w = clamp(distance(middlePointEdge3, eyePosition), minDistance, maxDistance);

	//Calculating the diference between the maximum and the
	//minimum distance
	float diffDistance = maxDistance - minDistance;

	//Calculating a tessellation factor from 0 to 1 for every
	//edge and the middle.
	//We apply the correction depending on the angle of the
	//camera.
	float2 factorInside = 1 - saturate(((magnitude - minDistance) / diffDistance) * correctionInside);
	float4 factorEdges = 1 - saturate(((magnitudeEdges - minDistance) / diffDistance) * correctionEdges);

	//Linear interpolation between the minimum and the maximum
	//tessellation exponent. Then we raised 2
	//to this value to get the tessellation factor.
	Output.Edges[0] = pow(2, lerp(minTessExp, maxTessExp, factorEdges.x));
	Output.Edges[1] = pow(2, lerp(minTessExp, maxTessExp, factorEdges.y));
	Output.Edges[2] = pow(2, lerp(minTessExp, maxTessExp, factorEdges.z));
	Output.Edges[3] = pow(2, lerp(minTessExp, maxTessExp, factorEdges.w));

	Output.Inside[0] = pow(2, lerp(minTessExp, maxTessExp, factorInside.x));
	Output.Inside[1] = pow(2, lerp(minTessExp, maxTessExp, factorInside.y));

	return Output;
}

// The hull shader is called once per output control point,
// which is specified with outputcontrolpoints. In this case,
// we take the control points from the vertex shader and pass
// them directly off to the domain shader.
// The input to the hull shader comes from the vertex shader.
// The output from the hull shader will go to the domain
// shader.
// The tessellation factor, topology, and partition mode will
// go to the fixed function tessellator stage to calculate the
// UVW and domain points.

[domain("quad")]
[partitioning(OCEAN_HS_PARTITION)]
[outputtopology("triangle_cw")]
[outputcontrolpoints(OUTPUT_PATCH_SIZE)]
[patchconstantfunc("OceanConstantHS")]
VERTEX_POSITION hsOcean(InputPatch<VERTEX_POSITION, INPUT_PATCH_SIZE> p, uint i : SV_OutputControlPointID, uint PatchID : SV_PrimitiveID)
{
	VERTEX_POSITION Output;
	Output.vPosition = p[i].vPosition;
	return Output;
}

