Texture2D Texture0;
SamplerState LinearSampler;
float fPercent;

void PSMain( in float2 tex : TexCoord0,
	out float4 outColor : SV_Target )
{
	float4 texColor = Texture0.Sample( LinearSampler, tex );
	float4 invTexColor = 1.0f - texColor;
	outColor = lerp( texColor, invTexColor, fPercent );
}