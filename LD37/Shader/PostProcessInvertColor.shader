Texture2D Texture0;
SamplerState LinearSampler;

void PSMain( in float4 inPos : SV_Position,
	in float2 tex : TexCoord0,
	out float4 outColor : SV_Target )
{
	float2 vTex = ( tex - 0.5f ) * 2;
	float l = length( vTex ) * 0.75f;
	vTex = vTex / l * asin( l ) * 0.88436922082490334587720703242994;
	float4 texColor = Texture0.Sample( LinearSampler, vTex * 0.5f + 0.5f );
	outColor = texColor;
}