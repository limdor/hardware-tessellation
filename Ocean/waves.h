//wave record
struct Wave
{
	float		wavelength;
	float		amplitude;
	float3		direction;
};

//wave data
#define NWAVES 8
Wave wave[NWAVES] = { {65.0f, 1.0f, float3(0.98, 0, 0.17) },
						{43.5f, 0.6f, float3(0.98, 0, -0.17) },
						{22.0f, 0.4f, float3(0.934, 0, 0.342) },
						{99.0f, 2.0f, float3(0.934, 0, -0.342) },
						{25.0f, 0.3f, float3(0.97, 0, 0.24) },
						{47.5f, 0.5f, float3(0.97, 0, -0.24) },
						{57.0f, 0.9f, float3(0.99, 0, -0.14) },
						{81.0f, 1.6f, float3(0.99, 0, 0.14) }
 };
