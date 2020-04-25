struct InOutVertex
{
	float2 tex : TexCoord0;
	float4 pos : SV_Position;
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