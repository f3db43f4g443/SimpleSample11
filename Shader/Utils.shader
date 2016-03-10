float4 DstRect;
float4 SrcRect;
float4 InvDstSrcResolution;

void VSScreen( in float2 pos : Position,
	out float2 outTex : TexCoord0,
	out float4 outPos : SV_Position )
{
	float2 vPos = ( pos * DstRect.zw + DstRect.xy ) * InvDstSrcResolution.xy;
	vPos = ( vPos * 2.0 - 1.0 ) * float2( 1.0, -1.0 );
	outPos = float4( vPos, 0, 1 );
	outTex = ( pos.xy * SrcRect.zw + SrcRect.xy ) * InvDstSrcResolution.zw;
}

float4 vColor;
void PSOneColor( in float2 tex : TexCoord0,
	out float4 outColor : SV_Target )
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

Texture2D Texture1;

void PSTwoTextureMultiply( in float2 tex : TexCoord0,
	out float4 outColor : SV_Target )
{
	outColor = Texture0.Sample( LinearSampler, tex ) * Texture1.Sample( LinearSampler, tex );
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