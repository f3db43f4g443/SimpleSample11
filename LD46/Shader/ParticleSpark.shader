float4x4 g_matView;
float g_zOrder;
float g_t;
float g_life;
float2 g_specialOfs;

cbuffer InstBuffer
{
	float4 g_insts[4096];
};

void VSParticle( in float2 tex : Position,
	in uint instID : SV_InstanceID,
	out float4 outPos : SV_Position,
	out float2 outTex : TexCoord0,
	out float4 outInstData : ExtraInstData0 )
{
	float2 pos = ( tex - float2( 0, 0.5 ) ) * float2( 1.0, -1.0 );
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
	float2 dir = v + a * t;
	float2 matX = normalize( dir );
	float2 matY = float2( -matX.y, matX.x );
	pos = pos.x * matX + pos.y * matY;
	pos = p0 + v * t + 0.5 * a * t * t + pos + g_specialOfs;

	outTex = tex;
	outInstData = float4( 1, 1, 1, 1 - t );
	outPos = mul( g_matView, float4( pos, instData.y, 1.0 ) );
}