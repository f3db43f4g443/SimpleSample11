#ifndef MAX_TEXTURES
#define MAX_TEXTURES 1
#endif

Texture2D g_texTarget[MAX_TEXTURES];
SamplerState g_samplerPointClamp;
float2 g_invViewportSize;

void PSMain( in float4 inPos : SV_Position,
	out float4 outColor[MAX_TEXTURES] : SV_Target0 )
{
	float2 tex = inPos.xy * g_invViewportSize;

	for( int i = 0; i < MAX_TEXTURES; i++ )
	{
		outColor[i] = g_texTarget[i].Sample( g_samplerPointClamp, tex );
	}
}