float4x4 g_matView;
float4 g_matWorld[2];
float4 g_rect;
float4 g_texRect;

void VSMain( in float2 tex : Position,
	out float2 outTex : TexCoord0,
	out float4 outPos : SV_Position )
{
	float2 pos = tex;
	pos.y = 1 - pos.y;
	pos = pos * g_rect.zw + g_rect.xy;

	pos.x = dot( float3( pos, 1 ), g_matWorld[0].xyz );
	pos.y = dot( float3( pos, 1 ), g_matWorld[1].xyz );

	outPos = mul( g_matView, float4( pos, g_matWorld[0].w, 1.0 ) );
	outTex = tex * g_texRect.zw + g_texRect.xy;;
}