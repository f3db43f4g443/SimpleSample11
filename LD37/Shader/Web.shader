float4x4 g_matView;
float g_zOrder;
float g_segmentsPerData;
float fTex2Scale;
float fTex2FadeScale;

cbuffer InstBuffer
{
	float4 g_insts[4096];
};

void VSWeb( in float2 tex : Position,
	in uint instID : SV_InstanceID,
	out float4 outPos : SV_Position,
	out float4 outTex : TexCoord0,
	out float4 outExtraData : ExtraData )
{
	float pos = tex.x * 2.0 - 1.0;
	float fracInstID = tex.y / g_segmentsPerData;

	float4 center_dir = lerp( g_insts[( uint )instID * 3], g_insts[( uint )instID * 3 + 3], fracInstID );

	float2 matX = center_dir.zw;
	center_dir.xy = center_dir.xy + pos * matX;
	outPos = mul( g_matView, float4( center_dir.xy, g_insts[( uint )instID * 3 + 1].w, 1.0 ) );

	float4 texData = lerp( g_insts[( uint )instID * 3 + 1], g_insts[( uint )instID * 3 + 4], fracInstID );
	outTex.x = tex.x;
	outTex.y = texData.z;
	outTex.z = texData.y;
	outTex.w = -normalize( matX ).y;
	outExtraData = g_insts[( uint )instID * 3 + 2];
}

float4 Coef;
float4 colortable[16];
float fColorTableSize;
float fTexXScale;

Texture2D Texture;
SamplerState Sampler;

void PSWeb( in float4 inPos : SV_Position,
	in float4 tex : TexCoord0,
	in float4 extraData : ExtraData,
	out float4 outColor[2] : SV_Target )
{
	float2 t0 = tex.xy;
	float2 w0 = 1.0 / float2( Coef.x + Coef.y * t0.y, Coef.z + Coef.w * ( tex.z - t0.y ) );
	float k = t0.y / tex.z;
	float w = lerp( w0.x, w0.y, k );
	float2 w1 = 1.0 / float2( Coef.x + Coef.y * t0.y * 0.5, Coef.z + Coef.w * ( tex.z - t0.y ) * 0.5 );
	float ofs = ( lerp( w1.x, w1.y, k ) - w ) * tex.w;

	float fColor1 = ( t0.x * 2 - 1 - ofs ) / w;
	fColor1 = min( 1, fColor1 * fColor1 );
	t0.x *= fTexXScale;
	t0 += extraData.zw;
	float fColor2 = Texture.Sample( Sampler, t0 ).x;
	float blendedColor = fColor1 + fColor2 - fColor1 * fColor2;
	blendedColor /= extraData.x;
	blendedColor = lerp( blendedColor, 1, extraData.y );

	int a = (int)floor( min( blendedColor, 0.9999 ) * fColorTableSize );
	outColor[1] = colortable[a];
	outColor[0] = float4( 0, 0, 0, outColor[1].w );
}