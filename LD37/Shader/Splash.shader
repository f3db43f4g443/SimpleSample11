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
	texColor += saturate( ( tex.y - fBlendBegin ) * fBlendScale );

	float fThreshold = 0.5;
	float4 color = texColor.x > fThreshold ? color0 : color1;
	clip( color.w - 0.5 );
	outColor[0] = 0;
	outColor[1] = color;
}

void PSOcclusion( in float2 tex : TexCoord0,
	out float4 outColor : SV_Target )
{
	float4 texColor = Texture0.Sample( Sampler, tex );
	texColor += saturate( ( tex.y - fBlendBegin ) * fBlendScale );

	float fThreshold = 0.5;
	float4 color = texColor.x > fThreshold ? color0 : color1;
	color.w = lerp( color1.w, color0.w, saturate( ( texColor.x - fThreshold ) / ( 1 - fThreshold ) ) );
	outColor = color;
}