Texture2D Texture0;
Texture2D Texture1;
Texture2D Texture2;
SamplerState LinearSampler;
float4 fParam;

void PSBlisterColor( in float2 tex : TexCoord0,
	out float4 outColor[2] : SV_Target )
{
	float fPercent = fParam.x;
	float k = fParam.y;
	float2 vDir = fParam.zw;
	float2 tex1 = tex * 2 - 1;
	float l = saturate( dot( tex1, vDir ) * 0.5 + 0.5 );
	float l1 = pow( l, k );
	tex += vDir * ( l1 - l );

	float fAlpha1 = saturate( fPercent * 2 - 1 );
	float fAlpha2 = saturate( fPercent * 2 );
	float4 texColor = Texture0.Sample( LinearSampler, tex );
	float4 texColor1 = Texture1.Sample( LinearSampler, tex );
	float4 texColor2 = Texture2.Sample( LinearSampler, tex );
	float4 color = lerp( texColor1, texColor2, texColor.w * fAlpha1 );
	color.w *= saturate( texColor.w * fAlpha2 + fAlpha2 );

	outColor[0] = color;
	outColor[1] = float4( 0, 0, 0, color.w );
	//clip( color.w - 0.001 );
}

float fBaseHeight;
float fHeightScale;
void PSBlisterOcclusion( in float2 tex : TexCoord0,
	out float4 outColor : SV_Target )
{
	float fPercent = fParam.x;
	float k = fParam.y;
	float2 vDir = fParam.zw;
	float2 tex1 = tex * 2 - 1;
	float l = saturate( dot( tex1, vDir ) * 0.5 + 0.5 );
	float l1 = pow( max( l, 0 ), k );
	tex += vDir * ( l1 - l );
	
	float fAlpha1 = saturate( fPercent * 2 - 1 );
	float4 texColor = Texture0.Sample( LinearSampler, tex );
	float4 texColor1 = Texture1.Sample( LinearSampler, tex );
	float4 color = texColor * texColor1 * fAlpha1;

	outColor = float4( 1, 1, 1, color.w * fHeightScale + fBaseHeight );
	//clip( color.w - 0.001 );
}