Texture2D Texture0;
SamplerState LinearSampler;
float4 Color0;
float4 Color1;

void PSTwoColorLerp( in float2 tex : TexCoord0,
	out float4 outColor : SV_Target )
{
	float4 texColor = Texture0.Sample( LinearSampler, tex );
	outColor.xyz = lerp( Color0.xyz, Color1.xyz, texColor.xyz );
	outColor.w = texColor.w;
}