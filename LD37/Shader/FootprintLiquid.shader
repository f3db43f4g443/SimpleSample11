Texture2D Texture0;
Texture2D TextureColorMap;
SamplerState LinearSampler;

void PSColor( in float4 inPos : SV_Position,
	in float2 tex : TexCoord0,
	out float4 outColor[2] : SV_Target )
{
	float4 texColor = Texture0.Sample( LinearSampler, tex );
	float4 color = TextureColorMap.Sample( LinearSampler, texColor.w );
	color.xyz *= texColor.xyz;

	outColor[0] = color;
	outColor[1] = float4( 0, 0, 0, color.w );
	//clip( color.w - 0.001 );
}

float fBaseHeight;
float fHeightScale;
void PSOcclusion( in float4 inPos : SV_Position,
	in float2 tex : TexCoord0,
	out float4 outColor : SV_Target )
{
	float4 texColor = Texture0.Sample( LinearSampler, tex );
	float4 color = TextureColorMap.Sample( LinearSampler, texColor.w );

	outColor = float4( 1, 1, 1, color.w * fHeightScale + fBaseHeight );
	//clip( color.w - 0.001 );
}

float4x4 g_matView;
float2 g_specialOfs;
float g_zOrder;
float g_t;
float g_life;
float3 sizeCurve;
float3 timeCurve;

cbuffer InstBuffer
{
	float4 g_insts[4096];
};

void VSParticle( in float2 tex : Position,
	in uint instID : SV_InstanceID,
	out float4 outPos : SV_Position,
	out float2 outTex : TexCoord0 )
{
	float2 pos = ( tex * 2.0 - 1.0 ) * float2( 1.0, -1.0 );
	float4 instData = g_insts[instID * 2];
	float4 instData1 = g_insts[instID * 2 + 1];
	float t = instData.x;
	t = ( g_t - t ) / g_life;
	t = t - floor( t );
	t = dot( timeCurve, float3( 1, t, t * t ) );

	float maxSize = instData.y;
	pos = pos * maxSize * dot( sizeCurve, float3( 1, t, t * t ) );
	float rot = instData.z * t + instData.w;
	float2 matX = float2( cos( rot ), sin( rot ) );
	float2 matY = float2( -matX.y, matX.x );
	pos = pos.x * matX + pos.y * matY;

	float2 center = instData1.xy;
	float2 dir = instData1.zw;
	pos = center + dir * t + pos + g_specialOfs;

	outTex = tex;
	outPos = mul( g_matView, float4( pos, g_zOrder, 1.0 ) );
}