float4x4 g_matView;
float fInvTexGrids;
float g_zOrder;
float g_t;
float g_life;
float g_sizeTimeScale;

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
	pos.y = ( -pos.y + 1.0 ) * fInvTexGrids;
	float4 instData = g_insts[instID * 2];
	float4 instData1 = g_insts[instID * 2 + 1];
	float t = instData.x;
	t = ( g_t - t ) / g_life;
	t = t - floor( t );

	float maxSize = instData.y;
	pos = pos * maxSize * min( t * g_sizeTimeScale, 1 );
	float2 center = instData.zw;
	float2 dir = instData1.xy;
	float texGrid = floor( instData1.z );

	float2 matX = normalize( dir );
	float2 matY = float2( -matX.y, matX.x );
	pos = pos.x * matX + pos.y * matY;
	pos = center + dir * t + pos;

	outTex.xy = tex;
	outTex.y = ( texGrid + outTex.y ) * fInvTexGrids;
	outTex.z = t * ( 2 - t );
	outPos = mul( g_matView, float4( pos, g_zOrder, 1.0 ) );
}

Texture2D Texture0;
Texture2D TextureColorMap;
SamplerState LinearSampler;
SamplerState LinearSampler1;

void PSColor( in float3 tex : TexCoord0,
	out float4 outColor[2] : SV_Target )
{
	float4 texColor = Texture0.Sample( LinearSampler, tex.xy );
	float alpha = saturate( texColor.w - tex.z );
	float4 color = TextureColorMap.Sample( LinearSampler1, alpha );
	color.xyz *= texColor.xyz;

	outColor[0] = color;
	outColor[1] = float4( 0, 0, 0, color.w );
}

float3 baseColor;
void PSOcclusion( in float3 tex : TexCoord0,
	out float4 outColor : SV_Target )
{
	float4 texColor = Texture0.Sample( LinearSampler, tex.xy );
	float alpha = saturate( texColor.w - tex.z );

	outColor = float4( lerp( 1, baseColor, alpha ), 0 );
}