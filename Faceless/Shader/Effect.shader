Texture2D Texture0;
SamplerState LinearSampler;
float4 Color0;
float4 Color1;

void PSTwoColorLerp( in float2 tex : TexCoord0,
	out float4 outColor : SV_Target )
{
	float4 texColor = Texture0.Sample( LinearSampler, tex );
	outColor.xyz = lerp( Color0.xyz, Color1.xyz, texColor.xyz );
	outColor.w = texColor.w;
}

float g_totalTime;
float timeScale;
float4x4 g_matView;
float g_zOrder;
float g_segmentsPerData;

cbuffer InstBuffer
{
	float4 g_insts[4096];
};

void VSSelectFaceTargetBullet( in float2 tex : Position,
	out float3 outTex : TexCoord0,
	out float4 outPos : SV_Position )
{
	float pos = tex.x * 2.0 - 1.0;
	float instID = tex.y / g_segmentsPerData;
	float nInstID = floor( instID );
	float fracInstID = instID - nInstID;
	nInstID = nInstID * 2;

	float4 center_dir = lerp( g_insts[( uint )nInstID], g_insts[( uint )nInstID + 2], fracInstID );
	float4 texData = lerp( g_insts[( uint )nInstID + 1], g_insts[( uint )nInstID + 3], fracInstID );

	float2 matX = center_dir.zw;
	center_dir.xy = center_dir.xy + pos * matX;
	outPos = mul( g_matView, float4( center_dir.xy, g_zOrder, 1.0 ) );

	outTex.xy = lerp( texData.xy, texData.zw, tex.x );
	outTex.z = pos;
}

void PSSelectFaceTargetBullet( in float3 tex : TexCoord0,
	out float4 outColor : SV_Target )
{
	float2 texCoord = tex.xy;
	texCoord.x = abs( texCoord.x );
	texCoord.y += texCoord.x;
	
	float c0 = frac( texCoord.y - g_totalTime * timeScale ) > 0.5 ? 1 : 0;
	float c1 = 1 - abs( tex.z );
	c1 = c1 * c1;
	outColor = Color0;
	outColor.w *= c0 * c1;
}