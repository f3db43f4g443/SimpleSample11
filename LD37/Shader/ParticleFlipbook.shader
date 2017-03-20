float4x4 g_matView;
float g_zOrder;
float g_t;
float g_life;
float2 g_specialOfs;

float g_columns;
float g_rows;
float g_frames;

cbuffer InstBuffer
{
	float4 g_insts[4096];
};

void VSParticle( in float2 tex : Position,
	in uint instID : SV_InstanceID,
	out float2 outTex : TexCoord0,
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

	float frameCount = floor( min( t * g_frames, g_frames - 0.01 ) );
	float row = floor( frameCount / g_columns );
	float column = frameCount - row * g_columns;
	outTex = ( tex + float2( column, row ) ) / float2( g_columns, g_rows );

	outPos = mul( g_matView, float4( pos, instData.y, 1.0 ) );
}
