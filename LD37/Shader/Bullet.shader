Texture2D Texture0;
Texture2D Texture1;
SamplerState Sampler;
SamplerState Sampler1;
float4 Color0;
float4 Color1;
float4 Color2;
float fWorldScale;

void PSBulletEffect( in float4 inPos : SV_Position,
	in float2 tex : TexCoord0,
	in float2 tex1 : TexCoord1,
	out float4 outColor[2] : SV_Target )
{
	float4 texColor = Texture0.Sample( Sampler, tex );
	float t = Texture1.Sample( Sampler1, tex1 * fWorldScale ).x;
	t = abs( t + texColor.x - 1 );
	outColor[0].xyz = 0;
	outColor[1].xyz = t < 0.2f ? lerp( Color0, Color1, t * 5 ) : lerp( Color1, Color2, t * 1.25f - 0.25f );
	outColor[0].w = outColor[1].w = texColor.w;
}