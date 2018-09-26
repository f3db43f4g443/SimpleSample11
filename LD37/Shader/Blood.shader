Texture2D Texture0;
SamplerState LinearSampler;

void PSBloodStain( in float4 inPos : SV_Position,
	in float2 tex : TexCoord0,
	in float4 outInstData : ExtraInstData0,
	out float4 outColor : SV_Target )
{
	float fAlphaScale = outInstData.x;
	float fAlphaOffset = outInstData.y;
	float4 texColor = Texture0.Sample( LinearSampler, tex );
	texColor.w = saturate( texColor.w * fAlphaScale + fAlphaOffset );
	outColor = texColor;
	outColor.xyz *= texColor.w;
}