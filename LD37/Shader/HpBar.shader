Texture2D Texture0;
SamplerState Sampler;

void PSHpBar( in float2 tex : TexCoord0,
	in float4 instData : ExtraInstData0,
	out float4 outColor : SV_Target )
{
	float fScale = instData.x;
	float fPercent = instData.y;
	float4 texColor = Texture0.Sample( Sampler, float2( tex.x * fScale, tex.y ) );
	
	outColor = texColor;
	if( tex.x > fPercent )
		outColor = 0;
}