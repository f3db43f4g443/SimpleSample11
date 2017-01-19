float4x4 g_matView;
float3 widthCurve;
float fTexYScale;
float fTexYTimeScale;
float fLength0;
float g_zOrder;
float g_segmentsPerData;
float g_t;

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
	out float2 outTex : TexCoord0,
	out float4 outPos : SV_Position )
{
	float pos = tex.x * 2.0 - 1.0;
	float instID = tex.y / g_segmentsPerData;
	float nInstID = floor( instID );
	float fracInstID = instID - nInstID;
	nInstID = nInstID * 2;

	float4 center_dir = lerp( g_insts[(uint)nInstID], g_insts[(uint)nInstID + 2], fracInstID );
	float4 texData = lerp( g_insts[(uint)nInstID + 1], g_insts[(uint)nInstID + 3], fracInstID );
	float4 particleInstData = g_insts1[particleInstID * 2];
	float4 particleInstData1 = g_insts1[particleInstID * 2 + 1];
	float t = g_t - particleInstData.x;
	float texYOfs = particleInstData.y;
	float tScale = particleInstData.z;
	float tOfs = particleInstData.w;

	float fLength = texData.y;
	float e = -1.0f / log2( fLength0 * instID / fLength );
	float fWidth = pow( instID, e );
	fWidth = lerp( 1, dot( widthCurve, float3( 1, fWidth, fWidth * fWidth ) ), particleInstData1.w );

	pos = pos * fWidth + ( 1 - fWidth ) * particleInstData1.z * sin( t * tScale + tOfs );
	float2 dir = center_dir.zw;
	center_dir.xy = center_dir.xy + particleInstData1.xy + pos * dir;
	outPos = mul( g_matView, float4( center_dir.xy, g_zOrder, 1.0 ) );

	outTex = lerp( texData.xy, texData.zw, tex.x );
	outTex.y = outTex.y / fTexYScale + fTexYTimeScale * t + texYOfs;
}