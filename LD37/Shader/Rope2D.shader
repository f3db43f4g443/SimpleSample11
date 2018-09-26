float4x4 g_matView;
float g_zOrder;
float g_segmentsPerData;

cbuffer InstBuffer
{
	float4 g_insts[4096];
};

void VSDefault( in float2 tex : Position,
	in uint instID : SV_InstanceID,
	out float4 outPos : SV_Position,
	out float2 outTex : TexCoord0 )
{
	float pos = tex.x * 2.0 - 1.0;
	float fracInstID = tex.y / g_segmentsPerData;

	float4 center_dir = lerp( g_insts[(uint)instID * 2], g_insts[(uint)instID * 2 + 2], fracInstID );

	float2 matX = center_dir.zw;
	center_dir.xy = center_dir.xy + pos * matX;
	outPos = mul( g_matView, float4( center_dir.xy, g_insts[(uint)instID * 2 + 1].w, 1.0 ) );

	float4 texData = lerp( g_insts[( uint )instID * 2 + 1], g_insts[( uint )instID * 2 + 3], fracInstID );
	outTex.x = lerp( texData.x, texData.y, tex.x );
	outTex.y = texData.z;
}

float4 g_staticData;
void VSDefaultStaticData1( in float2 tex : Position,
	in uint instID : SV_InstanceID,
	out float4 outPos : SV_Position,
	out float2 outTex : TexCoord0,
	out float4 outInstData : ExtraInstData0 )
{
	float pos = tex.x * 2.0 - 1.0;
	float fracInstID = tex.y / g_segmentsPerData;

	float4 center_dir = lerp( g_insts[( uint )instID * 2], g_insts[( uint )instID * 2 + 2], fracInstID );

	float2 matX = center_dir.zw;
	center_dir.xy = center_dir.xy + pos * matX;
	outPos = mul( g_matView, float4( center_dir.xy, g_insts[( uint )instID * 2 + 1].w, 1.0 ) );

	float4 texData = lerp( g_insts[( uint )instID * 2 + 1], g_insts[( uint )instID * 2 + 3], fracInstID );
	outTex.x = lerp( texData.x, texData.y, tex.x );
	outTex.y = texData.z;
	outInstData = g_staticData;
}