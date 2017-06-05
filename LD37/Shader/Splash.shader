Texture2D Texture0;
SamplerState Sampler;
float4 color0;
float4 color1;
float fBlendBegin;
float fBlendScale;

void PSColor( in float2 tex : TexCoord0,
	out float4 outColor[2] : SV_Target )
{
	float4 texColor = Texture0.Sample( Sampler, tex );
	float f = saturate( ( tex.y - fBlendBegin ) * fBlendScale );
	texColor += f * f;

	float fThreshold = 0.75;
	float4 color = texColor.x > fThreshold ? color0 : color1;
	clip( color.w - 0.5 );
	float fThreshold2 = 0.25;
	color.xyz = texColor.x < fThreshold2 ? color.xyz * 0.5 : color.xyz;
	outColor[0] = 0;
	outColor[1] = color;
}

void PSOcclusion( in float2 tex : TexCoord0,
	out float4 outColor : SV_Target )
{
	float4 texColor = Texture0.Sample( Sampler, tex );
	float f = saturate( ( tex.y - fBlendBegin ) * fBlendScale );
	texColor += f * f;

	float fThreshold = 0.75;
	float4 color = texColor.x > fThreshold ? color0 : color1;
	color.w = lerp( color1.w, color0.w, saturate( ( texColor.x - fThreshold ) / ( 1 - fThreshold ) ) );
	outColor = color;
}