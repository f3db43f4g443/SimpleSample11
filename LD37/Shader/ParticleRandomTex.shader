float4x4 g_matView;
float g_zOrder;
float g_t;
float g_life;
float2 g_specialOfs;

float2 g_texSize;
float2 g_align;
float2 g_scroll;

cbuffer InstBuffer
{
	float4 g_insts[4096];
};

void VSParticle( in float2 tex : Position,
	in uint instID : SV_InstanceID,
	out float2 outTex : TexCoord0,
	out float4 outInstData : ExtraInstData0,
	out float4 outPos : SV_Position )
{
	float2 pos = ( tex - 0.5 ) * float2( 1.0, -1.0 );
	float4 instData = g_insts[instID * 3];
	float4 instData1 = g_insts[instID * 3 + 1];
	float4 instData2 = g_insts[instID * 3 + 2];
	float t = instData.x;
	t = t / g_life;

	float2 size = instData.zw;
	pos = pos * size;

	float2 p0 = instData1.xy;
	float2 v = instData1.zw;
	float2 a = instData2.xy;
	pos = p0 + v * t + 0.5 * a * t * t + pos + g_specialOfs;

	outInstData = float4( tex.x, tex.y, 1, 1 - t );
	float2 randTex = instData2.zw;
	outTex = floor( ( randTex + t * g_scroll ) * g_align ) / g_align + tex * g_texSize;
	outPos = mul( g_matView, float4( pos, instData.y, 1.0 ) );
}

Texture2D Texture0;
Texture2D Texture1;
Texture2D TextureColorMap;
SamplerState LinearSampler;
SamplerState LinearSampler1;
SamplerState LinearSampler2;

void PSMask( in float2 tex : TexCoord0,
	in float4 instData : ExtraInstData0,
	out float4 outColor[2] : SV_Target )
{
	float4 texColor = Texture0.Sample( LinearSampler, tex );
	float mask = Texture1.Sample( LinearSampler1, instData.xy ).x;
	float t = instData.w;
	texColor.w = max( 0, ( texColor.w + mask - 1 ) * t );

	float4 color = TextureColorMap.Sample( LinearSampler2, texColor.w );
	color.xyz *= texColor.xyz;

	outColor[1] = color;
	outColor[0] = float4( 0, 0, 0, color.w );
}

void PSMaskOcclusion( in float2 tex : TexCoord0,
	in float4 instData : ExtraInstData0,
	out float4 outColor : SV_Target )
{
	float4 texColor = Texture0.Sample( LinearSampler, tex );
	float mask = Texture1.Sample( LinearSampler1, instData.xy ).x;
	float t = instData.w;
	texColor.w = max( 0, ( texColor.w + mask - 1 ) * t );

	float4 color = TextureColorMap.Sample( LinearSampler2, texColor.w );
	outColor = float4( color.xyz, 0 );
}