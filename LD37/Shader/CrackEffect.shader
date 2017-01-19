struct SVertexGenInfo
{
	float2 pos : Position;
	int nParent : DataIndex;
	float fArriveTime : TexCoord;
};

cbuffer VertGenInfoBuffer
{
	SVertexGenInfo vertInfo[4096];
};

struct SVertex
{
	float2 pos : Position;
	float2 center : Position1;
	float3 e1 : TexCoord0;
	float3 e2 : TexCoord1;
	float3 e3 : TexCoord2;
	float3 e4 : TexCoord3;
	float2 e1Time : TexCoord4;
	float2 e2Time : TexCoord5;
	float2 e3Time : TexCoord6;
	float2 e4Time : TexCoord7;
};

struct SVertexInVS
{
	float2 tex : Position;
};

struct SVertexInGS
{
	SVertexGenInfo info;
};

void VSGen( in SVertexInVS inVert, out SVertexInGS outVert )
{
	int index = int( inVert.tex.x + inVert.tex.y );
	outVert.info = vertInfo[inVert.tex.x + inVert.tex.y];
};

void LineFunc( in float2 p1, in float2 p2, out float3 e )
{
	e = float3( p1.y - p2.y, p2.x - p1.x, p1.x * p2.y - p1.y * p2.x ) / length( p2 - p1 );
}

float fSpeed;

[maxvertexcount(4)]
void GSGen( line SVertexInGS input[2], inout TriangleStream<SVertex> outStream )
{
	SVertexGenInfo parentData1 = vertInfo[input[0].info.nParent];
	SVertexGenInfo parentData2 = vertInfo[input[1].info.nParent];

	SVertex vert;
	LineFunc( parentData1.pos, input[0].info.pos, vert.e1 );
	LineFunc( parentData2.pos, input[1].info.pos, vert.e2 );
	LineFunc( input[0].info.pos, input[1].info.pos, vert.e4 );
	vert.e1Time = float2( parentData1.fArriveTime, input[0].info.fArriveTime );
	vert.e2Time = float2( parentData2.fArriveTime, input[1].info.fArriveTime );
	float l = length( parentData1.pos - parentData2.pos );
	vert.e3Time.x = min( parentData1.fArriveTime, parentData2.fArriveTime );
	vert.e3Time.y = min( vert.e3Time.x + l / fSpeed, ( l / fSpeed + parentData1.fArriveTime + parentData2.fArriveTime ) * 0.5 );
	l = length( input[0].info.pos - input[1].info.pos );
	vert.e4Time.x = min( input[0].info.fArriveTime, input[1].info.fArriveTime );
	vert.e4Time.y = min( vert.e4Time.x + l / fSpeed, ( l / fSpeed + input[0].info.fArriveTime + input[1].info.fArriveTime ) * 0.5 );
	vert.center = ( input[0].info.pos + input[1].info.pos + parentData1.pos + parentData2.pos ) * 0.25;

	if( input[0].info.nParent == input[1].info.nParent )
	{
		vert.e3 = float3( 0, 0, 10000000 );
		vert.pos = parentData1.pos;
		outStream.Append( vert );
		vert.pos = input[1].info.pos;
		outStream.Append( vert );
		vert.pos = input[0].info.pos;
		outStream.Append( vert );
		outStream.RestartStrip();
	}
	else
	{
		LineFunc( parentData1.pos, parentData2.pos, vert.e3 );
		vert.pos = parentData1.pos;
		outStream.Append( vert );
		vert.pos = parentData2.pos;
		outStream.Append( vert );
		vert.pos = input[0].info.pos;
		outStream.Append( vert );
		vert.pos = input[1].info.pos;
		outStream.Append( vert );
		outStream.RestartStrip();
	}
}

struct SVertexInPS
{
	float2 pos : Position;
	float2 tex : Tex;
	float4 e1 : TexCoord0;
	float4 e2 : TexCoord1;
	float4 e3 : TexCoord2;
	float4 e4 : TexCoord3;
};

float2 InvDstSrcResolution;
float fTime;
float fFlyTime;
float fFlySpeed;
float fRotSpeed;
void VSDraw( in SVertex inVert, out SVertexInPS outVert, out float4 outPos : SV_Position )
{
	outVert.pos = inVert.pos;
	float2 vPos = inVert.pos * InvDstSrcResolution * 2.0;
	outVert.tex = vPos * float2( 0.5, -0.5 ) + 0.5;

	outVert.e1.xyz = inVert.e1;
	outVert.e2.xyz = inVert.e2;
	outVert.e3.xyz = inVert.e3;
	outVert.e4.xyz = inVert.e4;
	outVert.e1.w = saturate( ( fTime - inVert.e1Time.x ) / ( inVert.e1Time.y - inVert.e1Time.x ) );
	outVert.e2.w = saturate( ( fTime - inVert.e2Time.x ) / ( inVert.e2Time.y - inVert.e2Time.x ) );
	outVert.e3.w = saturate( ( fTime - inVert.e3Time.x ) / ( inVert.e3Time.y - inVert.e3Time.x ) );
	outVert.e4.w = saturate( ( fTime - inVert.e4Time.x ) / ( inVert.e4Time.y - inVert.e4Time.x ) );

	vPos = inVert.pos;
	float fTime1 = max( fTime - inVert.e4Time.y - fFlyTime, 0 );
	float2 dir = normalize( inVert.center );
	float2 ofs = fTime1 * fFlySpeed * ( frac( ( inVert.center.x + inVert.center.y ) * 1.11 ) * 0.5 + 0.75 ) * dir;

	float fRot = fTime1 * ( fRotSpeed * frac( inVert.center.x + inVert.center.y ) );
	float2 localPos = vPos - inVert.center;
	localPos += dot( localPos, dir ) * ( cos( fRot ) - 1 ) * dir;

	vPos = localPos + inVert.center + ofs;
	vPos = vPos * InvDstSrcResolution * 2.0;
	outPos = float4( vPos, 0, 1 );
}

float3 baseColor;
Texture2D Texture0;
SamplerState LinearSampler;

float CalcColor( float3 pos, float4 e )
{
	float fDist = dot( pos, e.xyz ) * 0.1;
	fDist = saturate( 1 - abs( fDist ) );
	fDist = fDist * fDist;
	fDist = fDist * fDist;
	fDist = fDist * fDist;
	return fDist * e.w;
}

void PSDraw( in SVertexInPS inVert, out float4 outColor : SV_Target )
{
	float4 texColor = Texture0.Sample( LinearSampler, inVert.tex );

	float3 pos = float3( inVert.pos, 1 );
	float fColor = CalcColor( pos, inVert.e1 );
	fColor = max( fColor, CalcColor( pos, inVert.e2 ) );
	fColor = max( fColor, CalcColor( pos, inVert.e3 ) );
	fColor = max( fColor, CalcColor( pos, inVert.e4 ) );
	float3 color = baseColor * fColor;

	outColor.xyz = texColor.xyz + color - 2 * texColor.xyz * color;
	outColor.w = 1;
}