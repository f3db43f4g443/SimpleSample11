Texture2D Texture0;
SamplerState LinearSampler;

void PSDefault( in float4 inPos : SV_Position,
	in float2 tex : TexCoord0,
	out float4 outColor[3] : SV_Target )
{
	outColor[0] = Texture0.Sample( LinearSampler, tex );
	outColor[1] = float4( 0, 0, 0, outColor[0].w );
	outColor[2] = float4( 0, 0, 0, 0 );
	clip( outColor[0].w - 0.5 );
}

void PSDefaultMulInstData( in float4 inPos : SV_Position,
	in float2 tex : TexCoord0,
	in float4 instData : ExtraInstData0,
	out float4 outColor[3] : SV_Target )
{
	outColor[0] = Texture0.Sample( LinearSampler, tex ) * instData;
	outColor[1] = float4( 0, 0, 0, outColor[0].w );
	outColor[2] = float4( 0, 0, 0, 0 );
	clip( outColor[0].w - 0.5 );
}

void PSEmission( in float4 inPos : SV_Position,
	in float2 tex : TexCoord0,
	out float4 outColor[3] : SV_Target )
{
	outColor[0] = 0;
	outColor[1] = Texture0.Sample( LinearSampler, tex );
	outColor[2] = 0;
}

void PSEmissionMulInstData( in float4 inPos : SV_Position,
	in float2 tex : TexCoord0,
	in float4 instData : ExtraInstData0,
	out float4 outColor[3] : SV_Target )
{
	outColor[0] = 0;
	outColor[1] = Texture0.Sample( LinearSampler, tex ) * instData;
	outColor[2] = 0;
}

float PackNormal( in float2 norm )
{
	float2 n1 = clamp( norm * 7 + 8, 1, 15 );
	return ( n1.x * 16 + n1.y ) / 255;
}

Texture2D Texture1;
void PSGlass( in float4 inPos : SV_Position,
	in float2 tex : TexCoord0,
	out float4 outColor[3] : SV_Target )
{
	outColor[0] = Texture0.Sample( LinearSampler, tex );
	outColor[1] = float4( 0, 0, 0, outColor[0].w );
	float4 normMap = Texture1.Sample( LinearSampler, tex );
	outColor[2] = float4( PackNormal( normMap.xy * 2 - 1 ), normMap.z, normMap.w, 1 );
}