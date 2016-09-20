void VSDebugDraw( in float2 pos : Position,
	out float4 outPos : SV_Position )
{
	outPos = float4( pos, 0, 1 );
}

float4 DstRect;
float4 SrcRect;
float4 InvDstSrcResolution;
float fDepth;

void VSScreen( in float2 pos : Position,
	out float2 outTex : TexCoord0,
	out float4 outPos : SV_Position )
{
	float2 vPos = ( pos * DstRect.zw + DstRect.xy ) * InvDstSrcResolution.xy;
	vPos = ( vPos * 2.0 - 1.0 ) * float2( 1.0, -1.0 );
	outPos = float4( vPos, fDepth, 1 );
	outTex = ( pos.xy * SrcRect.zw + SrcRect.xy ) * InvDstSrcResolution.zw;
}

float4 vColor;
void PSOneColor( out float4 outColor : SV_Target )
{
	outColor = vColor;
}

Texture2D Texture0;
SamplerState LinearSampler;

void PSOneTexture( in float2 tex : TexCoord0,
	out float4 outColor : SV_Target )
{
	outColor = Texture0.Sample( LinearSampler, tex );
}

void PSOneTextureR( in float2 tex : TexCoord0,
	out float4 outColor : SV_Target )
{
	outColor = Texture0.Sample( LinearSampler, tex ).x;
}

void PSOneColorMulTextureAlpha( in float2 tex : TexCoord0,
	out float4 outColor : SV_Target )
{
	outColor = vColor;
	outColor.w *= Texture0.Sample( LinearSampler, tex ).x;
}

void PSInstData( in float2 tex : TexCoord0,
	in float4 instData : ExtraInstData0,
	out float4 outColor : SV_Target )
{
	outColor = instData;
}

void PSOneColorMulInstData( in float2 tex : TexCoord0,
	in float4 instData : ExtraInstData0,
	out float4 outColor : SV_Target )
{
	outColor = instData * vColor;
}

void PSOneTextureMulInstData( in float2 tex : TexCoord0,
	in float4 instData : ExtraInstData0,
	out float4 outColor : SV_Target )
{
	outColor = Texture0.Sample( LinearSampler, tex ) * instData;
}

Texture2D Texture1;

void PSTwoTextureMultiply( in float2 tex : TexCoord0,
	out float4 outColor : SV_Target )
{
	outColor = Texture0.Sample( LinearSampler, tex ) * Texture1.Sample( LinearSampler, tex );
}

void PSTwoTexCoordMasked( in float2 tex : TexCoord0,
	in float2 tex1 : TexCoord1,
	out float4 outColor : SV_Target )
{
	outColor = Texture0.Sample( LinearSampler, tex );
	outColor.w *= Texture1.Sample( LinearSampler, tex1 ).x;
}

#ifndef MAX_TEXTURES
#define MAX_TEXTURES 1
#endif
Texture2D Textures[MAX_TEXTURES];

void PSOneTexturePerRT( in float2 tex : TexCoord0,
	out float4 outColor[MAX_TEXTURES] : SV_Target0 )
{
	for( int i = 0; i < MAX_TEXTURES; i++ )
	{
		outColor[i] = Textures[i].Sample( LinearSampler, tex );
	}
}