Texture2D Texture0;
SamplerState LinearSampler;

void PSColorMat( in float4 inPos : SV_Position,
	in float2 tex : TexCoord0,
	in float4 instData : ExtraInstData0,
	in float4 instData1 : ExtraInstData1,
	in float4 instData2 : ExtraInstData2,
	out float4 outColor[2] : SV_Target )
{
	float fHeightClip = instData.z;
	float4 texColor = Texture0.Sample( LinearSampler, tex );
	float fAlpha = texColor.w;
	fAlpha = fAlpha - fHeightClip;

	texColor.xyz = float3( dot( texColor.xyz, instData1.xyz ),
		dot( texColor.xyz, instData2.xyz ),
		dot( texColor.xyz, float3( instData.w, instData1.w, instData2.w ) ) );

	outColor[0] = texColor;
	outColor[1] = float4( 0, 0, 0, outColor[0].w );
	clip( fAlpha - 0.001 );
}