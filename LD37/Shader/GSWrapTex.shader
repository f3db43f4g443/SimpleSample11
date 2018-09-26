struct InOutVertex
{
	float4 pos : SV_Position;
	float2 tex : TexCoord0;
};

[maxvertexcount(6)]
void GSMain( triangle InOutVertex input[3], inout TriangleStream<InOutVertex> outStream )
{
	float yMin = min( input[0].pos.y, min( input[1].pos.y, input[2].pos.y ) );
	float yMax = max( input[0].pos.y, max( input[1].pos.y, input[2].pos.y ) );
	float yMid = ( yMin + yMax ) * 0.5;
	float yOfs = yMid < 0? 2: -2;

	for( int v = 0; v < 3; v++ )
	{
		outStream.Append( input[v] );
	}
	outStream.RestartStrip();
	for( int v = 0; v < 3; v++ )
	{
		InOutVertex vert = input[v];
		vert.pos.y += yOfs;
		outStream.Append( vert );
	}
	outStream.RestartStrip();
}

[maxvertexcount( 6 )]
void GSMain1( triangle InOutVertex input[3], inout TriangleStream<InOutVertex> outStream )
{
	float xMin = min( input[0].pos.x, min( input[1].pos.x, input[2].pos.x ) );
	float xMax = max( input[0].pos.x, max( input[1].pos.x, input[2].pos.x ) );
	float xMid = ( xMin + xMax ) * 0.5;
	float xOfs = xMid < 0 ? 2 : -2;

	for( int v = 0; v < 3; v++ )
	{
		outStream.Append( input[v] );
	}
	outStream.RestartStrip();
	for( int v = 0; v < 3; v++ )
	{
		InOutVertex vert = input[v];
		vert.pos.x += xOfs;
		outStream.Append( vert );
	}
	outStream.RestartStrip();
}

[maxvertexcount( 12 )]
void GSMain2( triangle InOutVertex input[3], inout TriangleStream<InOutVertex> outStream )
{
	float xMin = min( input[0].pos.x, min( input[1].pos.x, input[2].pos.x ) );
	float xMax = max( input[0].pos.x, max( input[1].pos.x, input[2].pos.x ) );
	float xMid = ( xMin + xMax ) * 0.5;
	float xOfs = xMid < 0 ? 2 : -2;
	float yMin = min( input[0].pos.y, min( input[1].pos.y, input[2].pos.y ) );
	float yMax = max( input[0].pos.y, max( input[1].pos.y, input[2].pos.y ) );
	float yMid = ( yMin + yMax ) * 0.5;
	float yOfs = yMid < 0 ? 2 : -2;

	for( int v = 0; v < 3; v++ )
	{
		outStream.Append( input[v] );
	}
	outStream.RestartStrip();
	for( int v = 0; v < 3; v++ )
	{
		InOutVertex vert = input[v];
		vert.pos.x += xOfs;
		outStream.Append( vert );
	}
	outStream.RestartStrip();
	for( int v = 0; v < 3; v++ )
	{
		InOutVertex vert = input[v];
		vert.pos.y += yOfs;
		outStream.Append( vert );
	}
	outStream.RestartStrip();
	for( int v = 0; v < 3; v++ )
	{
		InOutVertex vert = input[v];
		vert.pos.x += xOfs;
		vert.pos.y += yOfs;
		outStream.Append( vert );
	}
	outStream.RestartStrip();
}