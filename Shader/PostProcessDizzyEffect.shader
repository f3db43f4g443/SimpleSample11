Texture2D Texture0;
SamplerState LinearSampler;
float fInvertPercent;

#define SAMPLE_COUNT 5
float4 texofs[SAMPLE_COUNT];
float4 weights[SAMPLE_COUNT];

float4 t[2];
float4 tx[2], ty[2];
float4 tb[2];

void PSMain( in float2 tex : TexCoord0,
	out float4 outColor : SV_Target )
{
	float f0 = dot( tb[0], cos( tex.x * tx[0] + tex.y * ty[0] + t[0] ) );
	float f1 = dot( tb[1], cos( tex.x * tx[1] + tex.y * ty[1] + t[1] ) );

	float4 texColor = 0;
	for( int i = 0; i < SAMPLE_COUNT; i++ )
	{
		texColor += Texture0.Sample( LinearSampler, tex + texofs[i].xy * f0 + texofs[i].zw * f1 ) * weights[i];
	}
	float4 invTexColor = 1.0f - texColor;
	outColor = lerp( texColor, invTexColor, fInvertPercent );
}