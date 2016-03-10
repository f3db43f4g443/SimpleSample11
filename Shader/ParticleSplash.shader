float4x4 g_matView;
float g_zOrder;
float g_t;
float g_life;

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
	t = t - floor( t );

	float maxSize = instData.y;
	pos = pos * maxSize;
	float2 center = instData.zw;
	float2 dir = instData1.xy;

	float2 matX = normalize( dir );
	float2 matY = float2( -matX.y, matX.x );
	pos = pos.x * matX + pos.y * matY;
	pos = center + dir * t + pos;

	outTex.xy = tex;
	outTex.z = 1 - t;
	outPos = mul( g_matView, float4( pos, g_zOrder, 1.0 ) );
}

float3 baseColor;
float3 baseColor1;
float fColorScale;
Texture2D Texture0;
SamplerState LinearSampler;

void PSColor( in float3 tex : TexCoord0,
	out float4 outColor[2] : SV_Target )
{
	float4 texColor = Texture0.Sample( LinearSampler, tex.xy );
	float alpha = texColor.x;
	alpha = alpha - tex.z;
	alpha = saturate( 1 - alpha * alpha * fColorScale ) * texColor.w;

	outColor[0] = float4( lerp( baseColor1, baseColor, alpha ), alpha );
	outColor[1] = float4( 0, 0, 0, outColor[0].w );
}

void PSOcclusion( in float3 tex : TexCoord0,
	out float4 outColor : SV_Target )
{
	float4 texColor = Texture0.Sample( LinearSampler, tex.xy );
	float alpha = texColor.x;
	alpha = alpha - tex.z;
	alpha = saturate( 1 - alpha * alpha * fColorScale ) * texColor.w;

	outColor = float4( lerp( 1, baseColor, alpha ), 0 );
}