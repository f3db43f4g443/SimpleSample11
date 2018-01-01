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
	out float2 outTex : TexCoord0,
	out float3 outTex1 : TexCoord1,
	out float4 outPos : SV_Position )
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

void PSThruster( in float2 tex : TexCoord0,
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