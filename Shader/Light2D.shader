Texture2D LightMap;
Texture2D ShadowMap;
Texture2D OcclusionMap;

SamplerState LinearSampler;

#ifndef MAX_OFFSET
#define MAX_OFFSET 1
#endif
float3 texOffset[MAX_OFFSET];

void PSLinearBlur( in float2 tex : TexCoord0,
	out float4 outShadow : SV_Target0 )
{
	float4 vShadow = ShadowMap.Sample( LinearSampler, tex );
	for( int i = 0; i < MAX_OFFSET; i++ )
	{
		float2 tex1 = tex + texOffset[i].xy;
		float4 vShadow1 = ShadowMap.Sample( LinearSampler, tex1 );
		float fDecay = texOffset[i].z;
		vShadow.xyz = min( vShadow.xyz, vShadow1.xyz + fDecay );
		vShadow.w = max( vShadow.w, vShadow1.w - fDecay );
	}

	outShadow = vShadow;
}

float2 RadialBlurCenter;
float RadialBlurBaseLength;
float LightHeight;
void PSRadialBlur( in float2 tex : TexCoord0,
	out float4 outShadow : SV_Target0 )
{
	float2 dTex = tex - RadialBlurCenter;
	float l = length( dTex );
	dTex = dTex / l;
	float4 vShadow = l > 0? ShadowMap.Sample( LinearSampler, tex ): float4( 1, 1, 1, 0 );

	for( int i = 1; i <= MAX_OFFSET; i++ )
	{
		float l1 = l - RadialBlurBaseLength * i;
		float2 tex1 = RadialBlurCenter + dTex * l1;
		float4 vShadow1 = ShadowMap.Sample( LinearSampler, tex1 );
		vShadow1 = l1 > 0? float4( vShadow1.xyz, min( vShadow1.w, LightHeight - ( LightHeight - vShadow1.w ) / l1 * l ) ): float4( 1, 1, 1, 0 );
		vShadow.xyz = min( vShadow.xyz, vShadow1.xyz );
		vShadow.w = max( vShadow.w, vShadow1.w );
	}
	
	outShadow = vShadow;
}

float4 LightAttenuationIntensity;
float fShadowScale;

void PSLinearSceneLighting( in float2 tex : TexCoord0,
	out float4 outLight : SV_Target0 )
{
	float4 vShadow = ShadowMap.Sample( LinearSampler, tex );
	float4 vOcclusion = OcclusionMap.Sample( LinearSampler, tex );
	vShadow.xyz *= saturate ( 1 + ( vOcclusion.w - vShadow.w ) * fShadowScale );
	outLight = float4( vShadow.xyz * LightAttenuationIntensity.w, 0 );
}

void PSRadialSceneLighting( in float2 tex : TexCoord0,
	out float4 outLight : SV_Target0 )
{
	float4 vShadow = ShadowMap.Sample( LinearSampler, tex );
	float4 vOcclusion = OcclusionMap.Sample( LinearSampler, tex );
	vShadow.xyz *= saturate ( 1 + ( vOcclusion.w - vShadow.w ) * fShadowScale );

	float2 dTex = tex - RadialBlurCenter;
	float l2 = dot( dTex, dTex );
	float fAttenuation = max( 0, 1.0 / dot( LightAttenuationIntensity.xyz, float3( 1, sqrt( l2 ), l2 ) ) + LightAttenuationIntensity.w );
	outLight = float4( vShadow.xyz * fAttenuation, 0 );
}

Texture2D ColorMap;
Texture2D EmissionMap;
void PSScene( in float2 tex : TexCoord0,
	out float4 outColor : SV_Target0 )
{
	outColor = ColorMap.Sample( LinearSampler, tex ) * LightMap.Sample( LinearSampler, tex ) + EmissionMap.Sample( LinearSampler, tex );
}