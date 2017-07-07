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
	float fRand = instData2.z;

	float frameCountRandom = floor( min( fRand * g_framesRandom, g_framesRandom - 0.01 ) );
	float rowRandom = floor( frameCountRandom / g_columnsRandom );
	float columnRandom = frameCountRandom - rowRandom * g_columnsRandom;

	outTex = ( tex + float2( column, row ) ) / float2( g_columns, g_rows );
	outTex = ( outTex + float2( columnRandom, rowRandom ) ) / float2( g_columnsRandom, g_rowsRandom );

	outPos = mul( g_matView, float4( pos, instData.y, 1.0 ) );
}

void VSParticleInstData( in float2 tex : Position,
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

	float frameCount = floor( min( t * g_frames, g_frames - 0.01 ) );
	float row = floor( frameCount / g_columns );
	float column = frameCount - row * g_columns;
	float fRand = instData2.z;

	float frameCountRandom = floor( min( fRand * g_framesRandom, g_framesRandom - 0.01 ) );
	float rowRandom = floor( frameCountRandom / g_columnsRandom );
	float columnRandom = frameCountRandom - rowRandom * g_columnsRandom;

	outTex = ( tex + float2( column, row ) ) / float2( g_columns, g_rows );
	outTex = ( outTex + float2( columnRandom, rowRandom ) ) / float2( g_columnsRandom, g_rowsRandom );
	outInstData = float4( 1, 1, 1, 1 - t );
	outPos = mul( g_matView, float4( pos, instData.y, 1.0 ) );
}

void VSParticle1( in float2 tex : Position,
	in uint instID : SV_InstanceID,
	out float2 outTex : TexCoord0,
	out float4 outPos : SV_Position )
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

float2 g_texScale;
float2 g_texOfs;
void VSParticle2( in float2 tex : Position,
	in uint instID : SV_InstanceID,
	out float2 outTex : TexCoord0,
	out float4 outInstData : ExtraInstData0,
	out float4 outPos : SV_Position )
{
	float2 pos = float2( tex.x, 1 - tex.y ) * g_texScale + g_texOfs;
	float4 instData = g_insts[instID * 3];
	float4 instData1 = g_insts[instID * 3 + 1];
	float4 instData2 = g_insts[instID * 3 + 2];
	float t = instData.x;
	t = t / g_life;

	float2 size = instData.zw;
	pos = pos * size * t;

	float2 p0 = instData1.xy;
	float2 v = instData1.zw;
	float2 a = instData2.xy;
	float2 dir = v + a * t;
	float2 matX = normalize( dir );
	float2 matY = float2( -matX.y, matX.x );
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
	outInstData = float4( 1, 1, 1, 1 - t );
	outPos = mul( g_matView, float4( pos, instData.y, 1.0 ) );
}

float4 g_matWorld[2];
void VSParticle2L( in float2 tex : Position,
	in uint instID : SV_InstanceID,
	out float2 outTex : TexCoord0,
	out float4 outInstData : ExtraInstData0,
	out float4 outPos : SV_Position )
{
	float2 pos = float2( tex.x, 1 - tex.y ) * g_texScale + g_texOfs;
	float4 instData = g_insts[instID * 3];
	float4 instData1 = g_insts[instID * 3 + 1];
	float4 instData2 = g_insts[instID * 3 + 2];
	float t = instData.x;
	t = ( g_t - t ) / g_life;
	t = t - floor( t );

	float2 size = instData.zw;
	pos = pos * size * t;

	float2 p0 = instData1.xy;
	float2 v = instData1.zw;
	float2 a = instData2.xy;
	float2 dir = v + a * t;
	float2 matX = normalize( dir );
	float2 matY = float2( -matX.y, matX.x );
	pos = pos.x * matX + pos.y * matY;
	pos = p0 + v * t + 0.5 * a * t * t + pos + g_specialOfs;
	pos.x = dot( float3( pos, 1 ), g_matWorld[0].xyz );
	pos.y = dot( float3( pos, 1 ), g_matWorld[1].xyz );

	float frameCount = floor( min( t * g_frames, g_frames - 0.01 ) );
	float row = floor( frameCount / g_columns );
	float column = frameCount - row * g_columns;
	float fRand = instData2.z;

	float frameCountRandom = floor( min( fRand * g_framesRandom, g_framesRandom - 0.01 ) );
	float rowRandom = floor( frameCountRandom / g_columnsRandom );
	float columnRandom = frameCountRandom - rowRandom * g_columnsRandom;

	outTex = ( tex + float2( column, row ) ) / float2( g_columns, g_rows );
	outTex = ( outTex + float2( columnRandom, rowRandom ) ) / float2( g_columnsRandom, g_rowsRandom );
	outInstData = float4( 1, 1, 1, 1 - t );
	outPos = mul( g_matView, float4( pos, g_zOrder, 1.0 ) );
}
