float4x4 g_matView;

cbuffer InstBuffer
{
	float4 g_insts[4096];
};

void Default2D_Vertex( in float2 tex,
	in uint instID,
	out float2 outTex,
	out float2 outTex1,
	out float4 outPos )
{
	float2 pos = ( tex * 2.0 - 1.0 ) * float2( 1.0, -1.0 );
	float4 center_mat = g_insts[instID];
	float4 tex_ratio_depth = g_insts[instID + 1];
	
	float2 matX = center_mat.zw;
	float2 matY = float2( -matX.y, matX.x ) * tex_ratio_depth.z;

	center_mat.xy = center_mat.xy + pos.x * matX;
	center_mat.xy = center_mat.xy + pos.y * matY;
	outPos = mul( g_matView, float4( center_mat.xy, tex_ratio_depth.w, 1.0 ) );
	
	float2 vtex = floor( tex_ratio_depth.xy );
	float2 dTex = 1.0 - ( tex_ratio_depth.xy - vtex );
	vtex = vtex / 2048.0;
	outTex = vtex + dTex * tex;
	outTex1 = outPos.xy * float2( 0.5, -0.5 ) + 0.5;
}

void VSDefault( in float2 tex : Position,
	in uint instID : SV_InstanceID,
	out float2 outTex : TexCoord0,
	out float2 outTex1 : TexCoord1,
	out float4 outPos : SV_Position )
{
	Default2D_Vertex( tex, instID * 2, outTex, outTex1, outPos );
}

#ifndef EXTRA_INST_DATA
#define EXTRA_INST_DATA 1
#endif
#define INST_DATA_SIZE ( 2 + EXTRA_INST_DATA )

void VSDefaultExtraInstData( in float2 tex : Position,
	in uint instID : SV_InstanceID,
	out float2 outTex : TexCoord0,
	out float2 outTex1 : TexCoord1,
	out float4 outInstData[EXTRA_INST_DATA] : ExtraInstData0,
	out float4 outPos : SV_Position )
{
	Default2D_Vertex( tex, instID * INST_DATA_SIZE, outTex, outTex1, outPos );

	for( int i = 0; i < EXTRA_INST_DATA; i++ )
		outInstData[i] = g_insts[instID * INST_DATA_SIZE + i + 2];
}