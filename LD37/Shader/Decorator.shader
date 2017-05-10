float fColorTableSize;
float4 colortable[16];

Texture2D Texture0;
Texture2D Texture1;
SamplerState Sampler;
SamplerState Sampler1;

void PSBlendVividLight( in float2 tex : TexCoord0,
	in float2 tex1 : TexCoord1,
	out float4 outColor : SV_Target )
{
	float texColor = Texture0.Sample( Sampler, tex ).x;

	float2 ddxtex1 = ddx( tex1 );
	float2 ddytex1 = ddy( tex1 );
	float2 dtex1 = ( ddxtex1 * ddxtex1 + ddytex1 * ddytex1 ) * 2;
	float2 fixedTex = floor( tex1 / dtex1 ) * dtex1;

	float texBlend = Texture1.Sample( Sampler1, fixedTex ).x;
	float blendedColor = clamp( texBlend > 0.5f ? texColor * 0.5f / ( 1 - texBlend ) : 1 + ( texColor - 1 ) * 0.5f / texBlend, 0, 0.999 );

	int a = (int)floor( blendedColor * fColorTableSize );
	outColor = colortable[a];
}