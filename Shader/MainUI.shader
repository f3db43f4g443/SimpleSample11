Texture2D Texture0;
Texture2D Texture1;
SamplerState LinearSampler;
float4 VignetteColor;

void PSMain( in float2 tex : TexCoord0,
	out float4 outColor : SV_Target )
{
	float2 vTex = ( tex - 0.5f ) * 2;
	float l = length( vTex ) * 0.5;
	vTex = vTex / l * asin( l );
	float4 texColor = Texture0.Sample( LinearSampler, vTex * 0.5f + 0.5f );

	float texColor1 = Texture1.Sample( LinearSampler, tex ).x * VignetteColor.w;
	vTex = abs( vTex );
	float fAlpha = max( vTex.x, vTex.y ) - 0.96;
	fAlpha = fAlpha * 9;
	fAlpha = saturate( saturate( ( texColor1 + fAlpha - 1 ) ) + l * 0.5 - 0.25 );
	fAlpha = fAlpha * fAlpha;

	outColor.xyz = VignetteColor.xyz * fAlpha + texColor.xyz;
	outColor.w = texColor.w + fAlpha - texColor.w * fAlpha;
}