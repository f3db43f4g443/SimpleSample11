float4x4 g_matView;
float g_zOrder;
float g_t;
float g_life;
float2 g_specialOfs;

float g_columns;
float g_rows;
float g_frames;
float g_columnsRandom;
float g_rowsRandom;
float g_framesRandom;

cbuffer InstBuffer
{
	float4 g_insts[4096];
};

float3 sizeCurve;
float3 timeCurve;
void VSParticle1Splash( in float2 tex : Position,
	in uint instID : SV_InstanceID,
	out float4 outPos : SV_Position,
	out float2 outTex : TexCoord0 )
{
	float2 pos = ( tex * 2.0 - 1.0 ) * float2( 1.0, -1.0 );
	float4 instData = g_insts[instID * 3];
	float4 instData1 = g_insts[instID * 3 + 1];
	float4 instData2 = g_insts[instID * 3 + 2];
	float t = instData.x;
	t = t / g_life;
	t = dot( timeCurve, float3( 1, t, t * t ) );

	float size = instData.z;
	pos = pos * size * dot( sizeCurve, float3( 1, t, t * t ) );
	float rot = instData.w;
	float2 matX = float2( cos( rot ), sin( rot ) );
	float2 matY = float2( -matX.y, matX.x );

	float2 p0 = instData1.xy;
	float2 v = instData1.zw;
	float2 a = instData2.xy;
	float2 dir = v + a * t;
	pos = pos.x * matX + pos.y * matY;
	pos = p0 + v * t + 0.5 * a * t * t + pos + g_specialOfs;

	float frameCount = floor( min( t * g_frames, g_frames - 0.01 ) );
	float row = floor( frameCount / g_columns );
	float column = frameCount - row * g_columns;
	float fRand = instData2.z;

	float frameCountRandom = floor( min( fRand * g_framesRandom, g_framesRandom - 0.01 ) );
	float rowRandom = floor( frameCountRandom / g_columnsRandom );
	float columnRandom = frameCountRandom - rowRandom * g_columnsRandom;

	outTex = ( tex + float2( column, row ) ) / float2( g_columns, g_rows );
	outTex = ( outTex + float2( columnRandom, rowRandom ) ) / float2( g_columnsRandom, g_rowsRandom );

	outPos = mul( g_matView, float4( pos, instData.y, 1.0 ) );
}
