float2 scrRes;

void VSDirectionalLightNoShadow( in float2 pos : Position,
	out float2 outTex : TexCoord0,
	out float2 texPos : TexCoord1,
	out float4 outPos : SV_Position )
{
	float2 vPos = pos;
	vPos = ( vPos * 2.0 - 1.0 ) * float2( 1.0, -1.0 );
	outPos = float4( vPos, 0, 1 );
	texPos = vPos * scrRes * 0.5f;
	outTex = pos;
}

float2 invResolution;
float2 lightCenter;
float lightRad;
void VSPointLightNoShadow( in float2 pos : Position,
	out float2 outTex : TexCoord0,
	out float2 texScr : TexCoord1,
	out float2 texPos : TexCoord2,
	out float4 outPos : SV_Position )
{
	float2 vPos = pos;
	vPos = lightCenter + ( vPos * 2.0 - 1.0 ) * float2( 1.0, -1.0 ) * lightRad;
	outPos = float4( vPos * invResolution * 2, 0, 1 );
	texPos = vPos;
	outTex = pos;
	texScr = vPos * invResolution + 0.5;
	texScr.y = 1 - texScr.y;
}

Texture2D ColorMap;
Texture2D NormalMap;
Texture2D LightMap;

SamplerState LinearSampler;
float4 lightData;
float3 lightColor;
float camHeight;
float3 lightPosDir;

float2 UnpackNormal( in float f )
{
	f = f * 255 / 16;
	return float2( ( floor( f ) - 8 ) / 7, ( frac( f ) * 16 - 8 ) / 7 );
}

void PSDirectionalLightNoShadow( in float2 texScr : TexCoord0,
	in float2 pos : TexCoord1,
	out float4 outLight : SV_Target0,
	out float4 outLight1 : SV_Target1 )
{
	outLight = float4( lightColor, 0 );

	float4 normGlossRefl = NormalMap.Sample( LinearSampler, texScr );
	float3 norm = float3( UnpackNormal( normGlossRefl.x ), 0 );
	norm.z = sqrt( max( 0, 1 - dot( norm.xy, norm.xy ) ) );
	float3 dl = -lightPosDir;
	float3 dc = float3( -pos, camHeight );
	dc = normalize( dc );
	float k = dot( normalize( dl + dc ), normalize( norm ) );
	k = pow( max( 0, k ), normGlossRefl.z * 100 + 0.0001 ) * normGlossRefl.y;

	outLight1 = outLight * k;
}

void PSPointLightNoShadow( in float4 tex : TexCoord0,
	in float2 texScr : TexCoord1,
	in float2 pos : TexCoord2,
	out float4 outLight : SV_Target0,
	out float4 outLight1 : SV_Target1 )
{
	float2 dTex = ( tex.xy - float2( 0.5, 0.5 ) ) * 2;
	float l2 = dot( dTex, dTex );
	float fAttenuation = max( 0, 1.0 / dot( lightData.xyz, float3( 1, sqrt( l2 ), l2 ) ) + lightData.w );
	outLight = float4( fAttenuation * lightColor, 0 );

	float4 normGlossRefl = NormalMap.Sample( LinearSampler, texScr );
	float3 norm = float3( UnpackNormal( normGlossRefl.x ), 0 );
	norm.z = sqrt( max( 0, 1 - dot( norm.xy, norm.xy ) ) );
	float3 dl = lightPosDir - float3( pos, 0 );
	float3 dc = float3( -pos, camHeight );
	dl = normalize( dl );
	dc = normalize( dc );
	float k = dot( normalize( dl + dc ), normalize( norm ) );
	k = pow( max( 0, k ), normGlossRefl.z * 100 + 0.0001 ) * normGlossRefl.y;

	outLight1 = outLight * k;
}

Texture2D EmissionMap;
void PSScene( in float4 inPos : SV_Position,
	in float2 tex : TexCoord0,
	out float4 outColor : SV_Target0 )
{
	outColor = ColorMap.Sample( LinearSampler, tex ) * LightMap.Sample( LinearSampler, tex ) + EmissionMap.Sample( LinearSampler, tex );
}