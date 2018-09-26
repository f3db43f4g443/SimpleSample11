float4x4 g_matView;
float4 g_matWorld[2];
float4 g_rect;
float4 g_texRect;

void VSMain( in float2 tex : Position,
	out float4 outPos : SV_Position,
	out float2 outTex : TexCoord0,
	out float2 outTex1 : TexCoord1 )
{
	float2 pos = tex;
	pos.y = 1 - pos.y;
	outTex1 = pos * 2 - 1;
	pos = pos * g_rect.zw + g_rect.xy;

	pos = float2( dot( float3( pos, 1 ), g_matWorld[0].xyz ),
		dot( float3( pos, 1 ), g_matWorld[1].xyz ) );

	outPos = mul( g_matView, float4( pos, g_matWorld[0].w, 1.0 ) );
	outTex = tex * g_texRect.zw + g_texRect.xy;
}

Texture2D Texture0;
SamplerState LinearSampler;
float4 vColor;
float4 vColor1;
float fRad;
float fWidth;

void PSMain( in float4 inPos : SV_Position,
	in float2 tex : TexCoord0,
	in float2 tex1 : TexCoord1,
	out float4 outColor : SV_Target )
{
	float4 texColor = Texture0.Sample( LinearSampler, tex );
	
	float l = length( tex1 );
	float alpha = saturate( 1 - abs( l * ( fRad + fWidth ) - fRad ) / fWidth );
	alpha = pow( alpha, 16 );
	float a = max( 2 * alpha - 1, 0 );
	float b = min( alpha * 2, 1 );
	alpha = lerp( a, b, texColor.x );

	float4 color = lerp( vColor, vColor1, alpha );
	outColor.xyz = color.xyz;
	outColor.w = color.w * alpha;
}

float2 dir;
float minDot;
void PSMainPercent( in float4 inPos : SV_Position,
	in float2 tex : TexCoord0,
	in float2 tex1 : TexCoord1,
	out float4 outColor : SV_Target )
{
	float4 texColor = Texture0.Sample( LinearSampler, tex );
	
	float l = length( tex1 );
	float alpha = saturate( 1 - abs( l * ( fRad + fWidth ) - fRad ) / fWidth );
	alpha = pow( alpha, 16 );

	float2 vDir = normalize( tex1 );
	alpha = alpha * ( 1 - step( dot( dir, vDir ), minDot ) );

	float a = max( 2 * alpha - 1, 0 );
	float b = min( alpha * 2, 1 );
	alpha = lerp( a, b, texColor.x );

	float4 color = lerp( vColor, vColor1, alpha );
	outColor.xyz = color.xyz;
	outColor.w = color.w * alpha;
}