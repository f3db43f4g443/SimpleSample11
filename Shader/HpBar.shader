float4x4 g_matView;

cbuffer InstBuffer
{
	float4 g_insts[4096];
};

void VSHpBar( in float2 pos : Position,
	in uint instID : SV_InstanceID,
	out float3 outTex : TexCoord0,
	out float4 outColor : TexCoord1,
	out float4 outDstRect : TexCoord2,
	out float4 outPos : SV_Position )
{
	float4 DstRect = g_insts[instID * 3];
	float4 params = g_insts[instID * 3 + 1];
	float fSkew = params.x;
	float fBlur = params.y;
	float fDepth = params.z;
	outColor = g_insts[instID * 3 + 2];

	float2 vPos = pos;
	vPos.y = 1 - vPos.y;
	vPos = ( vPos * DstRect.zw + DstRect.xy );
	vPos.x += pos.y * fSkew.x * DstRect.w;
	outPos = mul( g_matView, float4( vPos, fDepth, 1.0 ) );

	outTex.xy = pos - 0.5;
	outTex.z = fBlur;
	outDstRect = DstRect;
}

void PSHpBar( in float3 tex : TexCoord0,
	in float4 color : TexCoord1,
	in float4 DstRect : TexCoord2,
	out float4 outColor : SV_Target )
{
	float2 vTex = abs( tex.xy );
	float fBlur = tex.z;
	float2 vBlur = 1.0 - saturate( ( 0.5 - vTex ) * DstRect.zw / fBlur );
	float f = saturate( 1.0 - length( vBlur ) );
	f = pow( f, 0.3 );
	outColor = color * f;
}

void PSHpBarBorder( in float3 tex : TexCoord0,
	in float4 color : TexCoord1,
	in float4 DstRect : TexCoord2,
	out float4 outColor : SV_Target )
{
	float2 vTex = abs( tex.xy );
	float fBlur = tex.z;
	float2 fDist = ( 0.5 - vTex ) * DstRect.zw / fBlur;
	float2 vBlur = saturate( 1.0 - fDist );
	float2 vBlur1 = saturate( 2.0 - fDist );
	float2 f = float2( saturate( length( vBlur ) ), saturate( 1.0 - length( vBlur1 ) ) );
	f = pow( f, 0.3 );
	outColor = color * ( 1.0 - f.x ) * ( 1.0 - f.y );
}