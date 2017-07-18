Texture2D LightMap;
Texture2D ColorMap;
Texture2D EmissionMap;
Texture2D OcclusionMap;
Texture2D TransmissionMap;

SamplerState PointSampler;
SamplerState PointSampler1;
SamplerState LinearSampler;

float4 DstRect;
float4 SrcRect;
float4 SrcRect1;
float4 SrcRect2;
float4 InvDstSrcResolution;
float2 InvSrc1Resolution;
float2 InvSrc2Resolution;
float fDepth;

void VSPreTransmission( in float2 pos : Position,
	out float2 outTex : TexCoord0,
	out float2 outTex1 : TexCoord1,
	out float2 outTex2 : TexCoord2,
	out float4 outPos : SV_Position )
{
	float2 vPos = ( pos * DstRect.zw + DstRect.xy ) * InvDstSrcResolution.xy;
	vPos = ( vPos * 2.0 - 1.0 ) * float2( 1.0, -1.0 );
	outPos = float4( vPos, fDepth, 1 );
	outTex = ( pos.xy * SrcRect.zw + SrcRect.xy ) * InvDstSrcResolution.zw;
	outTex1 = ( pos.xy * SrcRect1.zw + SrcRect1.xy ) * InvSrc1Resolution;
	outTex2 = ( pos.xy * SrcRect2.zw + SrcRect2.xy ) * InvSrc2Resolution;
}

void PSPreTransmission( in float2 tex : TexCoord0, //color
	in float2 tex1 : TexCoord1, //occlusion
	in float2 tex2 : TexCoord2, //transmission
	out float4 outScene : SV_Target0,
	out float4 outTrans : SV_Target1,
	out float4 outTransTemp : SV_Target2 )
{
	float4 LightMapTex = LightMap.Sample( PointSampler, tex );
	float4 ColorMapTex = ColorMap.Sample( PointSampler, tex );
	float4 EmissionMapTex = EmissionMap.Sample( PointSampler, tex );
	float4 TransmissionMapTex = TransmissionMap.Sample( PointSampler, tex2 );
	float4 OcclusionMapTex = OcclusionMap.Sample( PointSampler, tex1 );

	float fPercent = 0.8;
	float fLInject1 = 0.65;
	float fEInject1 = 1;
	float fLightMapCoef = 0.35;

	outScene.xyz = ( LightMapTex.xyz * fLightMapCoef + TransmissionMapTex.xyz ) * ColorMapTex.xyz + EmissionMapTex.xyz;
	outScene.w = 1;

	outTrans = TransmissionMapTex;
	outTransTemp.xyz = TransmissionMapTex.xyz * fPercent + ( LightMapTex.xyz * fLInject1 + EmissionMapTex.xyz * ( fEInject1 + ( 1 - OcclusionMapTex.xyz ) * ( 1 - fEInject1 ) ) ) * ( 1 - fPercent );
	outTrans.w = outTransTemp.w = OcclusionMapTex.w;
}

void VSTransmission( in float2 pos : Position,
	out float2 outTex : TexCoord0,
	out float2 outTex1 : TexCoord1,
	out float4 outPos : SV_Position )
{
	float2 vPos = ( pos * DstRect.zw + DstRect.xy ) * InvDstSrcResolution.xy;
	vPos = ( vPos * 2.0 - 1.0 ) * float2( 1.0, -1.0 );
	outPos = float4( vPos, fDepth, 1 );
	outTex = ( pos.xy * SrcRect.zw + SrcRect.xy ) * InvDstSrcResolution.zw;
	outTex1 = ( pos.xy * SrcRect1.zw + SrcRect1.xy ) * InvSrc1Resolution;
}

Texture2D TransmissionTemp;
Texture2D RandomNormalMap;
float2 InvSrcRes;

void PSTransmission( in float2 tex : TexCoord0, //color
	in float2 tex1 : TexCoord1, //randomnormal
	out float4 outTransmission : SV_Target0 )
{
	float2 dTex = InvSrcRes.xy;
	float4 TransmissionMapTex = TransmissionMap.Sample( PointSampler, tex );

	float fMaxRad = 8.0f;
	float2 ofsTex = ( RandomNormalMap.Sample( PointSampler1, tex1 ).xy * 2 - 1 ) * fMaxRad;
	float2 ofs[4] = { float2( ofsTex.x, ofsTex.y ), float2( -ofsTex.y, ofsTex.x ), float2( -ofsTex.x, -ofsTex.y ), float2( ofsTex.y, -ofsTex.x ) };
	float4 TransmissionTempTex[4];
	for( int i = 0; i < 4; i++ )
		TransmissionTempTex[i] = TransmissionTemp.Sample( LinearSampler, tex + ofs[i] * dTex );

	float2 dHeight = float2( TransmissionTempTex[0].w - TransmissionTempTex[2].w, TransmissionTempTex[1].w - TransmissionTempTex[3].w );
	dHeight *= 0.5;

	float hScale = 10.0;
	float hScale1 = 10.0;

	float fDecay = 0.2;
	float fWeight = ( 1 - fDecay ) * 0.25;
	float fWeight1 = 0;
	outTransmission.xyz = 0;

	float2 ofs1[4] = { float2( 1, 0 ), float2( 0, 1 ), float2( -1, 0 ), float2( 0, -1 ) };
	for( int i = 0; i < 4; i++ )
	{
		float h0 = dot( dHeight, ofs1[i] ) + TransmissionMapTex.w;
		float h1 = TransmissionTempTex[i].w;
		float w = saturate( 1 - ( TransmissionMapTex.w - h1 ) * hScale1 ) * fWeight;
		fWeight1 += w;
		outTransmission.xyz += TransmissionTempTex[i].xyz * saturate( 1 - ( h1 - h0 ) * hScale ) * w;
	}
	outTransmission.xyz += TransmissionMapTex.xyz * ( 1 - fWeight1 );
	outTransmission.w = 1;
}