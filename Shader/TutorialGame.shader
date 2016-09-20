Texture2D Texture0;
Texture2D Texture1;
SamplerState PointSampler;
SamplerState LinearSampler;
float fBaseAlpha;

void PSMain( in float2 tex : TexCoord0,
	out float4 outColor : SV_Target )
{
	float2 vTex = ( tex - 0.5f ) * 2;
	float l = length( vTex ) * 0.5;
	float k = asin( l ) / l;
	vTex = vTex * k;
	float4 texColor = Texture0.Sample( LinearSampler, vTex * 0.5f + 0.5f );

	float4 texColor1 = Texture1.Sample( LinearSampler, tex );
	float fAlpha = min( fBaseAlpha * k, 1 );

	outColor.xyz = lerp( texColor.xyz, texColor1.xyz, texColor1.w );
	outColor.w = texColor1.w + fAlpha - texColor1.w * fAlpha;
}

float g_totalTime;
float fScanLineWidth;
float2 vGamma;
float4 vColor;
float4 vTimeScale;
float4 vTimeCoef;

//Scanline pass

void PSScanLineBase( in float2 tex : TexCoord0,
	in float2 tex1 : TexCoord1,
	out float4 outColor : SV_Target )
{
	outColor = 1;
	outColor.x = frac( floor( tex1.y / fScanLineWidth ) * 0.5 ) < 0.5 ? 0 : 1;
	float fTimeCoef = dot( abs( cos( vTimeScale * g_totalTime ) ), vTimeCoef );
	outColor.x = lerp( 0.5f, outColor.x, fTimeCoef );
}

void PSScanLineBase1( in float2 tex : TexCoord0,
	in float2 tex1 : TexCoord1,
	in float4 instData : ExtraInstData0,
	out float4 outColor : SV_Target )
{
	outColor = 1;
	outColor.x = ( frac( floor( tex1.x / fScanLineWidth ) * 0.25 ) < 0.25 ? 0 : 1 )
		* ( frac( floor( tex1.y / fScanLineWidth ) * 0.25 ) < 0.25 ? 0 : 1 );
	float fTimeCoef = dot( abs( cos( vTimeScale * g_totalTime ) ), vTimeCoef );
	float fPosCoef = length( tex * 2 - 1 ) <= instData.x ? 1 : 0;
	outColor.x = lerp( 0.5f, outColor.x, fTimeCoef );
	outColor.w = fPosCoef;
}

//Color pass

float ScanLineAlpha( in float alpha, in float fScanLine )
{
	return pow( alpha, pow( vGamma.x, fScanLine ) * pow( vGamma.y, 1 - fScanLine ) );
}

float3 ScanLineColor( in float3 color, in float fScanLine )
{
	return pow( color, pow( vGamma.x, fScanLine ) * pow( vGamma.y, 1 - fScanLine ) );
}

void PSScanLine( in float2 tex : TexCoord0,
	in float2 tex1 : TexCoord1,
	out float4 outColor : SV_Target )
{
	outColor = vColor;
	float alpha = Texture0.Sample( LinearSampler, tex ).x;
	float4 effectColor = Texture1.Sample( LinearSampler, tex1 );
	alpha = ScanLineAlpha( alpha, effectColor.x );
	outColor.w *= alpha;
}

float4 vColor1;
void PSScanLine1( in float2 tex : TexCoord0,
	in float2 tex1 : TexCoord1,
	out float4 outColor : SV_Target )
{
	float alpha = Texture0.Sample( LinearSampler, tex ).x;
	float4 effectColor = Texture1.Sample( LinearSampler, tex1 );
	outColor = lerp( vColor, vColor1, effectColor.x ) * alpha;
}

float fColorTimeScale;
void PSFlash( in float2 tex : TexCoord0,
	in float2 tex1 : TexCoord1,
	out float4 outColor : SV_Target )
{
	float4 texColor = Texture0.Sample( PointSampler, tex );
	float4 effectColor = Texture1.Sample( LinearSampler, tex1 ).x;
	float l = ScanLineAlpha( texColor.x, effectColor.x );
	float h = frac( texColor.y + g_totalTime * fColorTimeScale ) * 3;
	float3 color = float3( saturate( max( 1 - h, h - 2 ) * 2 ),
		saturate( min( 2 - h, h ) * 2 ),
		saturate( min( 3 - h, h - 1 ) * 2 ) );
	color = l > 0.5 ? lerp( color, 1, l * 2 - 1 ) : lerp( 0, color, l * 2 );

	outColor = float4( color, texColor.w );
}

float2 g_screenResolution;
void PSScrCoordMasked( in float2 tex : TexCoord0,
	in float2 tex1 : TexCoord1,
	in float4 instData : ExtraInstData0,
	out float4 outColor : SV_Target )
{
	outColor = Texture0.Sample( LinearSampler, tex );
	float2 tesScr = ( ( tex1 - 0.5 ) * g_screenResolution - instData.xy ) / instData.zw;
	outColor.w *= Texture1.Sample( LinearSampler, tesScr ).x;
	outColor.xyz *= outColor.w;
}

void PSHREffect( in float2 tex : TexCoord0,
	in float2 tex1 : TexCoord1,
	in float4 instData : ExtraInstData0,
	out float4 outColor : SV_Target )
{
	outColor = Texture0.Sample( LinearSampler, tex1 * g_screenResolution / instData.yz );
	float4 effectColor = Texture1.Sample( LinearSampler, tex1 );
	outColor.xyz = ScanLineColor( outColor.xyz, effectColor.x );
	float fPosCoef = length( tex * 2 - 1 ) <= instData.x ? 1 : 0;
	outColor *= fPosCoef;
}