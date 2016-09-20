Texture2D Texture0;
SamplerState LinearSampler;
float3 dirX, dirY, minDot, radBegin, radEnd;

void PSMain( in float2 tex : TexCoord0,
	out float4 outColor : SV_Target )
{
	float4 texColor = Texture0.Sample( LinearSampler, tex );

	float2 ofs = tex * 2 - 1;
	float rad = length( ofs );
	float2 dir = normalize( ofs );

	float3 vDot = dir.x * dirX + dir.y * dirY;
	float3 color = step( vDot, minDot );
	float t = saturate( ( rad - radBegin ) / ( radEnd - radBegin ) );
	color = lerp( 1.0, color, t );

	outColor = texColor;
	outColor.xyz *= color;
}