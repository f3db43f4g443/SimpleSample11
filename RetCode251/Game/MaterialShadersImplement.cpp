#include "stdafx.h"
#include "Material.h"

void Game_ShaderImplement_Dummy_Effect();
void Game_ShaderImplement_Dummy_PostEffects();
void Game_ShaderImplement_Dummy()
{
	Engine_ShaderImplement_Dummy();
	Game_ShaderImplement_Dummy_Effect();
	Game_ShaderImplement_Dummy_PostEffects();
}

IMPLEMENT_MATERIAL_SHADER( PSDistortion, "Shader/Distortion.shader", "PSDistortion", "ps_5_0" );
IMPLEMENT_MATERIAL_SHADER( PSDistortionAdd, "Shader/Distortion.shader", "PSDistortionAdd", "ps_5_0" );
IMPLEMENT_MATERIAL_SHADER( PSColorAdjust, "Shader/Distortion.shader", "PSColorAdjust", "ps_5_0" );

IMPLEMENT_MATERIAL_SHADER( PSEmissionClearColorClip, "Shader/Effect.shader", "PSEmissionClearColorClip", "ps_5_0" );
IMPLEMENT_MATERIAL_SHADER( PSEmissionClearColorNoClip, "Shader/Effect.shader", "PSEmissionClearColorNoClip", "ps_5_0" );
IMPLEMENT_MATERIAL_SHADER( PSPhantomEffect, "Shader/Effect.shader", "PSPhantomEffect", "ps_5_0" );
IMPLEMENT_MATERIAL_SHADER( PSCircle, "Shader/misc.shader", "PSCircle", "ps_5_0" );
IMPLEMENT_MATERIAL_SHADER( VSParticleCommon, "Shader/ParticleCommon.shader", "VSParticle", "vs_5_0" );
IMPLEMENT_MATERIAL_SHADER( VSParticle, "Shader/Particle.shader", "VSParticle", "vs_5_0" );
IMPLEMENT_MATERIAL_SHADER( VSParticleLocal, "Shader/Particle.shader", "VSParticle_local", "vs_5_0" );
IMPLEMENT_MATERIAL_SHADER( VSParticle1, "Shader/Particle1.shader", "VSParticle", "vs_5_0" );
IMPLEMENT_MATERIAL_SHADER( PSParticle1, "Shader/Particle1.shader", "PSParticle", "ps_5_0" );
IMPLEMENT_MATERIAL_SHADER( VSParticleSplash, "Shader/ParticleSplash.shader", "VSParticle", "vs_5_0" );
IMPLEMENT_MATERIAL_SHADER( PSParticleSplashColor, "Shader/ParticleSplash.shader", "PSColor", "ps_5_0" );
IMPLEMENT_MATERIAL_SHADER( PSParticleSplashOcclusion, "Shader/ParticleSplash.shader", "PSOcclusion", "ps_5_0" );
IMPLEMENT_MATERIAL_SHADER( GSWrapTex, "Shader/GSWrapTex.shader", "GSMain", "gs_5_0" );

IMPLEMENT_MATERIAL_SHADER( PSWater, "Shader/Water.shader", "PSWater", "ps_5_0" );
IMPLEMENT_MATERIAL_SHADER( PSWaterOneColor, "Shader/Water.shader", "PSWaterOneColor", "ps_5_0" );