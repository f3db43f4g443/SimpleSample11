float4x4 g_matView;
float g_zOrder;
float g_t;
float g_life;
float2 g_specialOfs;
float2 g_texSize;

cbuffer InstBuffer
{
	float4 g_insts[4096];
};

void VSParticleGlitch( in float2 tex : Position,
	in uint instID : SV_InstanceID,
	out float2 outTex : TexCoord0,
	out float4 outPos : SV_Position )
{
	float2 pos = ( tex - 0.5 ) * float2( 1.0, -1.0 );
	float4 instData = g_insts[instID * 2];
	float4 instData1 = g_insts[instID * 2 + 1];
	float t = instData.x;
	t = t / g_life;

	float2 size = instData.zw;
	pos = pos * size;

	float2 p0 = instData1.xy;
	float2 texOfs = instData1.zw;
	pos = p0 + pos + g_specialOfs;

	outTex = tex * size / g_texSize + texOfs;
	outPos = mul( g_matView, float4( pos, instData.y, 1.0 ) );
}
