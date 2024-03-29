float fColorTableSize;
float4 colortable[16];

Texture2D Texture0;
SamplerState Sampler;

void PSColorTable( in float4 inPos : SV_Position,
	in float2 tex : TexCoord0,
	out float4 outColor[2] : SV_Target )
{
	float texColor = Texture0.Sample( Sampler, tex ).x;
	int a = (int)floor( min( 0.999, texColor ) * fColorTableSize );
	outColor[1] = colortable[a];
	outColor[0] = float4( 0, 0, 0, outColor[1].w );
}

Texture2D Texture1;
SamplerState Sampler1;

void PSBlendVividLight( in float4 inPos : SV_Position,
	in float2 tex : TexCoord0,
	in float2 tex1 : TexCoord1,
	out float4 outColor : SV_Target )
{
	float texColor = Texture0.Sample( Sampler, tex ).x;

	float2 ddxtex1 = ddx( tex1 );
	float2 ddytex1 = ddy( tex1 );
	float2 dtex1 = sqrt( ddxtex1 * ddxtex1 + ddytex1 * ddytex1 ) * 2;
	float2 fixedTex = floor( tex1 / dtex1 ) * dtex1;

	float texBlend = Texture1.Sample( Sampler1, fixedTex ).x;
	float blendedColor = clamp( texBlend > 0.5f ? texColor * 0.5f / ( 1 - texBlend ) : 1 + ( texColor - 1 ) * 0.5f / texBlend, 0, 0.999 );

	int a = (int)floor( blendedColor * fColorTableSize );
	outColor = colortable[a];
}


void PSBlendMul( in float4 inPos : SV_Position,
	in float2 tex : TexCoord0,
	in float2 tex1 : TexCoord1,
	in float4 instData : ExtraInstData0,
	out float4 outColor : SV_Target )
{
	float texColor = Texture0.Sample( Sampler, tex ).x;

	tex1 = instData.xy + instData.zw * tex1;
	float2 ddxtex1 = ddx( tex1 );
	float2 ddytex1 = ddy( tex1 );
	float2 dtex1 = sqrt( ddxtex1 * ddxtex1 + ddytex1 * ddytex1 ) * 2;
	float2 fixedTex = floor( tex1 / dtex1 ) * dtex1;

	float texBlend = Texture1.Sample( Sampler1, fixedTex ).x;
	float blendedColor = clamp( texBlend * texColor, 0, 0.999 );

	int a = (int)floor( blendedColor * fColorTableSize );
	outColor = colortable[a];
}