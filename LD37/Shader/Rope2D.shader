float4x4 g_matView;
float g_zOrder;
float g_segmentsPerData;

cbuffer InstBuffer
{
	float4 g_insts[4096];
};

void VSDefault( in float2 tex : Position,
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

	float2 matX = center_dir.zw;
	center_dir.xy = center_dir.xy + pos * matX;
	outPos = mul( g_matView, float4( center_dir.xy, g_zOrder, 1.0 ) );

	outTex = lerp( texData.xy, texData.zw, tex.x );
}