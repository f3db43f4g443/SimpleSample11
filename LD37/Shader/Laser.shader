float4x4 g_matView;
float fTexYTimeScale;
float fInvTexGrids;
float2 fTex1Scale;
float g_zOrder;
float g_segmentsPerData;
float g_t;
float g_life;

cbuffer InstBuffer
{
	float4 g_insts[4096];
};

cbuffer InstBuffer1
{
	float4 g_insts1[4096];
};

void VSParticle( in float2 tex : Position,
	in uint particleInstID : SV_InstanceID,
	out float4 outPos : SV_Position,
	out float3 outTex : TexCoord0,
	out float2 outTex1 : TexCoord1 )
{
	float pos = tex.x * 2.0 - 1.0;
	float instID = tex.y / g_segmentsPerData;
	float nInstID = floor( instID );
	float fracInstID = instID - nInstID;
	nInstID = nInstID * 2;

	float4 center_dir = lerp( g_insts[(uint)nInstID], g_insts[(uint)nInstID + 2], fracInstID );
	float4 texData = lerp( g_insts[(uint)nInstID + 1], g_insts[(uint)nInstID + 3], fracInstID );
	float4 particleInstData = g_insts1[particleInstID];
	float t = particleInstData.x;
	t = ( g_t - t ) / g_life;
	t = t - floor( t );
	float texYOfs = particleInstData.y;
	float width = particleInstData.z * t * ( 2 - t );
	float texGrid = floor( particleInstData.w );

	outTex1.x = pos * 1.001;
	outTex1.y = texData.z * lerp( fTex1Scale.x, fTex1Scale.y, t ) - 1;

	pos = pos * width;
	float2 dir = center_dir.zw;
	center_dir.xy = center_dir.xy + pos * dir;
	outPos = mul( g_matView, float4( center_dir.xy, g_zOrder, 1.0 ) );

	outTex.x = lerp( texData.x, texData.y, tex.x );
	outTex.y = texData.z;
	outTex.x = ( texGrid + outTex.x ) * fInvTexGrids;
	outTex.y = outTex.y + fTexYTimeScale * t * ( 2 - t ) + texYOfs;
	outTex.z = 1 - t;

}

float fTexYTimeScale1;
void VSParticle1( in float2 tex : Position,
	in uint particleInstID : SV_InstanceID,
	out float4 outPos : SV_Position,
	out float2 outTex : TexCoord0,
	out float4 outInstData : ExtraInstData0 )
{
	float pos = tex.x * 2.0 - 1.0;
	float instID = tex.y / g_segmentsPerData;
	float nInstID = floor( instID );
	float fracInstID = instID - nInstID;
	nInstID = nInstID * 2;

	float4 center_dir = lerp( g_insts[( uint )nInstID], g_insts[( uint )nInstID + 2], fracInstID );
	float4 texData = lerp( g_insts[( uint )nInstID + 1], g_insts[( uint )nInstID + 3], fracInstID );
	float4 particleInstData = g_insts1[particleInstID * 2];
	float4 particleInstData1 = g_insts1[particleInstID * 2 + 1];
	float t = particleInstData.x;
	t = ( g_t - t ) / g_life;
	t = t - floor( t );
	float texYOfs = particleInstData.y;
	float width = particleInstData.z;
	float texGrid = floor( particleInstData.w );
	float xOfs = particleInstData1.x;
	float2 randTex = particleInstData1.zw;

	pos = pos * width + xOfs;
	float2 dir = center_dir.zw;
	center_dir.xy = center_dir.xy + pos * dir;
	outPos = mul( g_matView, float4( center_dir.xy, g_zOrder, 1.0 ) );

	outTex.x = lerp( texData.x, texData.y, tex.x );
	outTex.y = texData.z;
	outTex.x = ( texGrid + outTex.x ) * fInvTexGrids;
	outTex.y = outTex.y + fTexYTimeScale * t * ( 2 - t ) + texYOfs;

	outInstData = float4( randTex.x + tex.x * fTex1Scale.x, randTex.y + texData.z * fTex1Scale.y, texData.w, 1 - t );
}

Texture2D Texture0;
Texture2D TextureColorMap;
SamplerState LinearSampler;
SamplerState LinearSampler1;

void PSParticle( in float4 inPos : SV_Position,
	in float3 tex : TexCoord0,
	in float2 outTex1 : TexCoord1,
	out float4 outColor[2] : SV_Target )
{
	float2 l = outTex1;
	float a = 1 - l.x * l.x;
	float b = sqrt( a );
	float k = saturate( ( l.y + b ) / ( b - a ) );

	float4 texColor = Texture0.Sample( LinearSampler, tex.xy );
	texColor.w *= tex.z * k;
	float4 color = TextureColorMap.Sample( LinearSampler1, texColor.w );
	color.xyz *= texColor.xyz;

	outColor[1] = color;
	outColor[0] = float4( 0, 0, 0, color.w );
}

void PSParticleOcclusion( in float4 inPos : SV_Position,
	in float3 tex : TexCoord0,
	in float2 outTex1 : TexCoord1,
	out float4 outColor : SV_Target )
{
	float2 l = outTex1;
	float a = 1 - l.x * l.x;
	float b = sqrt( a );
	float k = saturate( ( l.y + b ) / ( b - a ) );

	float4 texColor = Texture0.Sample( LinearSampler, tex.xy );
	texColor.w *= tex.z * k;
	float4 color = TextureColorMap.Sample( LinearSampler1, texColor.w );
	outColor = float4( color.xyz, 0 );
}