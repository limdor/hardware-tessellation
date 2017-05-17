//------------------------------------------------------------
// Vertex shader section
//------------------------------------------------------------
struct VERTEX_POSITION
{
	float3 vPosition	: POSITION;
};

// This vertex shader passes the control points straight
// through to the hull shader.
// The input to the vertex shader comes from the vertex
// buffer.
// The output from the vertex shader will go into the hull
// shader.

VERTEX_POSITION vsOcean(VERTEX_POSITION Input)
{
	VERTEX_POSITION Output;

	Output.vPosition = Input.vPosition;

	return Output;
}
