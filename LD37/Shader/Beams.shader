float4x4 g_matView;
float g_zOrder;
float g_segmentsPerData;
float fTex2Scale;
float fTex2FadeScale;

cbuffer InstBuffer
{
	float4 g_insts[4096];
};

void VSThruster( in float2 tex : Position,
	in uint instID : SV_InstanceID,
	out float4 outPos : SV_Position,
	out float2 outTex : TexCoord0,
	out float3 outTex1 : TexCoord1 )
{
	float pos = tex.x * 2.0 - 1.0;
	float fracInstID = tex.y / g_segmentsPerData;

	float4 center_dir = lerp( g_insts[( uint )instID * 2], g_insts[( uint )instID * 2 + 2], fracInstID );

	float2 matX = center_dir.zw;
	center_dir.xy = center_dir.xy + pos * matX;
	outPos = mul( g_matView, float4( center_dir.xy, g_insts[( uint )instID * 2 + 1].w, 1.0 ) );

	float4 texData = lerp( g_insts[( uint )instID * 2 + 1], g_insts[( uint )instID * 2 + 3], fracInstID );
	outTex.x = tex.x;
	outTex.y = texData.z;
	outTex1.x = tex.x;
	outTex1.y = ( texData.y - texData.z ) * fTex2Scale;
	outTex1.z = saturate( ( 1 - texData.y ) * fTex2FadeScale );
}

float4 colortable[16];
float4 vTimeScale;
float4 vTimeCoef;
float fColorTableSize;
float fGamma;
float g_totalTime;

Texture2D Texture0;
Texture2D Texture1;
SamplerState Sampler;
SamplerState Sampler1;

void PSThruster( in float4 inPos : SV_Position,
	in float2 tex : TexCoord0,
	in float3 tex1 : TexCoord1,
	out float4 outColor[2] : SV_Target )
{
	float texColor = Texture0.Sample( Sampler, tex ).x;
	float texBlend = Texture1.Sample( Sampler1, tex1.xy ).x;
	float fTimeCoef = dot( cos( vTimeScale * g_totalTime ), vTimeCoef );
	float blendedColor = texColor + texBlend * tex1.z;
	blendedColor = pow( blendedColor, pow( fGamma, fTimeCoef ) );

	int a = (int)floor( min( blendedColor, 0.9999 ) * fColorTableSize );
	outColor[1] = colortable[a];
	outColor[0] = float4( 0, 0, 0, outColor[1].w );
}

void VSThruster1( in float2 tex : Position,
	in uint instID : SV_InstanceID,
	out float4 outPos : SV_Position,
	out float2 outTex : TexCoord0,
	out float2 outTex1 : TexCoord1,
	out float4 outExtraData : ExtraData )
{
	float pos = tex.x * 2.0 - 1.0;
	float fracInstID = tex.y / g_segmentsPerData;

	float4 center_dir = lerp( g_insts[( uint )instID * 3], g_insts[( uint )instID * 3 + 3], fracInstID );

	float2 matX = center_dir.zw;
	center_dir.xy = center_dir.xy + pos * matX;
	outPos = mul( g_matView, float4( center_dir.xy, g_insts[( uint )instID * 3 + 1].w, 1.0 ) );

	float4 texData = lerp( g_insts[( uint )instID * 3 + 1], g_insts[( uint )instID * 3 + 4], fracInstID );
	outTex.x = tex.x;
	outTex.y = texData.z;
	outTex1.x = tex.x;
	outTex1.y = 1 - ( texData.y - texData.z ) * fTex2Scale;
	outExtraData = g_insts[( uint )instID * 3 + 2];
}

float fVel;
float fTex1Scale;
float fFade;
float fTexAlign0;
float fTexAlignTarget;
float2 g_invViewportSize;
float4 distortiontable[16];

Texture2D Texture2;
SamplerState Sampler2;
Texture2D g_texTarget0;
Texture2D g_texTarget1;
SamplerState g_samplerPointClamp;

void PSThruster1Color( in float4 inPos : SV_Position,
	in float2 tex : TexCoord0,
	in float2 tex1 : TexCoord1,
	in float4 extraData : ExtraData,
	out float4 outColor[2] : SV_Target )
{
	float fNrml0 = 128.0 / 255.0;
	float4 texBase = Texture0.Sample( Sampler, tex + float2( 0, floor( g_totalTime * fVel * fTexAlign0 ) / fTexAlign0 ) );
	float4 texMask1 = Texture1.Sample( Sampler1, tex * float2( 1, fTex1Scale ) );
	float4 texMask2 = Texture2.Sample( Sampler2, tex1.xy );
	float4 texBlended = texBase + texMask1;
	texBlended.yz -= fNrml0 * 2;
	texBlended = texBlended * ( 1 - texMask2.w );
	texBlended.x += texMask2.x;
	texBlended *= min( 1, ( 1 - abs( 0.5 - tex.x ) * 2 ) * fFade );

	float fFadeIn = extraData.x;
	float fFadeOut = extraData.y;
	float fFadeInPos = extraData.z;
	float fFadeOutPos = extraData.w;
	float y = floor( tex.y * fTexAlign0 + 0.5 ) / fTexAlign0;
	fFadeIn = saturate( ( fFadeInPos - y ) * fFadeIn );
	fFadeOut = saturate( ( fFadeOutPos - y ) * fFadeOut );
	texBlended.x *= fFadeIn;
	texBlended.x = texBlended.x + fFadeOut - texBlended.x * fFadeOut;

	int a = (int)floor( min( texBlended.x, 0.9999 ) * fColorTableSize );
	float4 color = colortable[a];
	float2 texTarget = inPos.xy;
	float2 texOfs = texBlended.yz * distortiontable[a].x * 2;
	texOfs = floor( texOfs / fTexAlignTarget + 0.5 ) * fTexAlignTarget;
	texTarget += texOfs;
	texTarget *= g_invViewportSize;

	float4 dst0 = g_texTarget0.Sample( g_samplerPointClamp, texTarget );
	float4 dst1 = g_texTarget1.Sample( g_samplerPointClamp, texTarget );
	outColor[0].xyz = dst0.xyz * ( 1 - color.w );
	outColor[0].w = 1;
	outColor[1].xyz = color.xyz + dst1.xyz * ( 1 - color.w );
	outColor[1].w = 1;
}

void PSThruster1Occ( in float4 inPos : SV_Position,
	in float2 tex : TexCoord0,
	in float2 tex1 : TexCoord1,
	in float4 extraData : ExtraData,
	out float4 outColor : SV_Target )
{
	float fNrml0 = 128.0 / 255.0;
	float4 texBase = Texture0.Sample( Sampler, tex + float2( 0, floor( g_totalTime * fVel * fTexAlign0 ) / fTexAlign0 ) );
	float4 texMask1 = Texture1.Sample( Sampler1, tex * float2( 1, fTex1Scale ) );
	float4 texMask2 = Texture2.Sample( Sampler2, tex1.xy );
	float4 texBlended = texBase + texMask1;
	texBlended.yz -= fNrml0 * 2;
	texBlended = texBlended * ( 1 - texMask2.w );
	texBlended.x += texMask2.x;
	texBlended *= min( 1, ( 1 - abs( 0.5 - tex.x ) * 2 ) * fFade );

	float fFadeIn = extraData.x;
	float fFadeOut = extraData.y;
	float fFadeInPos = extraData.z;
	float fFadeOutPos = extraData.w;
	float y = floor( tex.y * fTexAlign0 + 0.5 ) / fTexAlign0;
	fFadeIn = saturate( ( fFadeInPos - y ) * fFadeIn );
	fFadeOut = saturate( ( fFadeOutPos - y ) * fFadeOut );
	texBlended.x *= fFadeIn;
	texBlended.x = texBlended.x + fFadeOut - texBlended.x * fFadeOut;

	int a = (int)floor( min( texBlended.x, 0.9999 ) * fColorTableSize );
	float4 color = colortable[a];
	float2 texTarget = inPos.xy;
	float2 texOfs = texBlended.yz * distortiontable[a].x * 2;
	texOfs = floor( texOfs / fTexAlignTarget + 0.5 ) * fTexAlignTarget;
	texTarget += texOfs;
	texTarget *= g_invViewportSize;

	float4 dst0 = g_texTarget0.Sample( g_samplerPointClamp, texTarget );
	outColor.xyz = min( dst0.xyz, color.xyz );
	outColor.w = dst0.w;
}