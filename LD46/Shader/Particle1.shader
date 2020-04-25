float4x4 g_matView;
float g_zOrder;
float g_t;
float g_life;
float fInvTexGrids;

cbuffer InstBuffer
{
	float4 g_insts[4096];
};

void VSParticle( in float2 tex : Position,
	in uint instID : SV_InstanceID,
	out float3 outTex : TexCoord0,
	out float4 outPos : SV_Position )
{
	float2 pos = tex * 2.0;
	pos.y = -pos.y + 1.0;
	float4 instData = g_insts[instID * 2];
	float4 instData1 = g_insts[instID * 2 + 1];
	float t = instData.x;
	t = ( g_t - t ) / g_life;
	t = t * ( 2 - t );

	float2 maxSize = instData.yz;
	pos = pos * maxSize * ( 1 - t );
	float texGrid = floor( instData.w );
	float2 center = instData1.xy;
	float2 dir = instData1.zw;

	float2 matX = normalize( dir );
	float2 matY = float2( -matX.y, matX.x );
	pos = pos.x * matX + pos.y * matY;
	pos = center + dir * ( 1 - t ) + pos;

	outTex.xy = tex;
	outTex.y = ( texGrid + outTex.y ) * fInvTexGrids;
	outTex.z = t;
	outPos = mul( g_matView, float4( pos, g_zOrder, 1.0 ) );
}

Texture2D Texture0;
SamplerState LinearSampler;

void PSParticle( in float3 tex : TexCoord0,
	out float4 outColor : SV_Target )
{
	float4 texColor = Texture0.Sample( LinearSampler, tex.xy ) * tex.z;
	outColor = texColor;
}