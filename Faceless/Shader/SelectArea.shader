float g_totalTime;
float timeScale;

Texture2D Texture0;
SamplerState LinearSampler;

void PSFaceSelectArea( in float2 tex : TexCoord0,
	in float4 instData : ExtraInstData0,
	out float4 outColor : SV_Target )
{
	float4 texColor = Texture0.Sample( LinearSampler, tex );
	float f = frac( g_totalTime * timeScale ) > 0.5f ? 0 : 1;
	outColor = instData;
	outColor.w *= texColor.x + texColor.y * f + texColor.z * ( 1 - f );
}