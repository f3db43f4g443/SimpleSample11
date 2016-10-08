#include "Default2D.shh"

void VSDefault( in float2 tex : Position,
	in uint instID : SV_InstanceID,
	out float2 outTex : TexCoord0,
	out float4 outPos : SV_Position )
{
	Default2D_Vertex( tex, instID * 2, outTex, outPos );
}

#ifndef EXTRA_INST_DATA
#define EXTRA_INST_DATA 1
#endif
#define INST_DATA_SIZE ( 2 + EXTRA_INST_DATA )

void VSDefaultExtraInstData( in float2 tex : Position,
	in uint instID : SV_InstanceID,
	out float2 outTex : TexCoord0,
	out float4 outInstData[EXTRA_INST_DATA] : ExtraInstData0,
	out float4 outPos : SV_Position )
{
	Default2D_Vertex( tex, instID * INST_DATA_SIZE, outTex, outPos );

	for( int i = 0; i < EXTRA_INST_DATA; i++ )
		outInstData[i] = g_insts[instID * INST_DATA_SIZE + i + 2];
}