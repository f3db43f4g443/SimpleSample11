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
	out float2 outTex : TexCoord0,
	out float4 outPos : SV_Position )
{
	float2 pos = ( tex * 2.0 - 1.0 ) * float2( 1.0, -1.0 );
	float4 instData = g_insts[instID * 2];
	float4 instData1 = g_insts[instID * 2 + 1];
	float t = instData.x;
	t = t / g_life;

	float maxSize = instData.y;
	pos = pos * maxSize * ( 1 - t );
	float rot = instData.w;
	float2 matX = float2( cos( rot ), sin( rot ) );
	float2 matY = float2( -matX.y, matX.x );
	pos = pos.x * matX + pos.y * matY;

	float2 center = instData1.xy;
	float2 dir = instData1.zw;
	pos = center + dir * t + pos + g_specialOfs;

	outTex = tex;
	outPos = mul( g_matView, float4( pos, instData.z, 1.0 ) );
}

float4 g_matWorld[2];

void VSParticle_local( in float2 tex : Position,
	in uint instID : SV_InstanceID,
	out float2 outTex : TexCoord0,
	out float4 outPos : SV_Position )
{
	float2 pos = ( tex * 2.0 - 1.0 ) * float2( 1.0, -1.0 );
	float4 instData = g_insts[instID * 2];
	float4 instData1 = g_insts[instID * 2 + 1];
	float t = instData.x;
	t = ( g_t - t ) / g_life;

	float maxSize = instData.y;
	pos = pos * maxSize * ( 1 - t );
	float rot = instData.w;
	float2 matX = float2( cos( rot ), sin( rot ) );
	float2 matY = float2( -matX.y, matX.x );
	pos = pos.x * matX + pos.y * matY;

	float2 center = instData1.xy;
	float2 dir = instData1.zw;
	pos = center + dir * t + pos + g_specialOfs;

	pos = float2( dot( float3( pos, 1 ), g_matWorld[0].xyz ),
		dot( float3( pos, 1 ), g_matWorld[1].xyz ) );

	outTex = tex;
	outPos = mul( g_matView, float4( pos, g_zOrder, 1.0 ) );
}