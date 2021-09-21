#include "stdafx.h"
#include "Material.h"

void Game_ShaderImplement_Dummy_PostEffects();
void Game_ShaderImplement_Dummy()
{
	Engine_ShaderImplement_Dummy();
	Game_ShaderImplement_Dummy_PostEffects();
}

IMPLEMENT_MATERIAL_SHADER( PSActionEffect, "Shader/Effect.shader", "PSActionEffect", "ps_5_0" );
IMPLEMENT_MATERIAL_SHADER( PSTextureInstDataClip, "Shader/Effect.shader", "PSTextureInstDataClip", "ps_5_0" );
IMPLEMENT_MATERIAL_SHADER( PSTextureColorRange, "Shader/Effect.shader", "PSTextureColorRange", "ps_5_0" );
IMPLEMENT_MATERIAL_SHADER( PSChargeEffect, "Shader/Effect.shader", "PSChargeEffect", "ps_5_0" );
IMPLEMENT_MATERIAL_SHADER( PSRemap, "Shader/Distortion.shader", "PSRemap", "ps_5_0" );
IMPLEMENT_MATERIAL_SHADER( PSDistortion, "Shader/Distortion.shader", "PSDistortion", "ps_5_0" );
IMPLEMENT_MATERIAL_SHADER( PSDistortionAdd, "Shader/Distortion.shader", "PSDistortionAdd", "ps_5_0" );
IMPLEMENT_MATERIAL_SHADER( PSDistortionMasked, "Shader/Distortion.shader", "PSDistortionMasked", "ps_5_0" );
IMPLEMENT_MATERIAL_SHADER( PSDistortionEffect, "Shader/Distortion.shader", "PSDistortionEffect", "ps_5_0" );
IMPLEMENT_MATERIAL_SHADER( PSDistortionEffect1, "Shader/Distortion.shader", "PSDistortionEffect1", "ps_5_0" );
IMPLEMENT_MATERIAL_SHADER( PSColorAdjust, "Shader/Distortion.shader", "PSColorAdjust", "ps_5_0" );
IMPLEMENT_MATERIAL_SHADER( VSParticleCommon, "Shader/ParticleCommon.shader", "VSParticle", "vs_5_0" );
IMPLEMENT_MATERIAL_SHADER( VSParticle, "Shader/Particle.shader", "VSParticle", "vs_5_0" );
IMPLEMENT_MATERIAL_SHADER( VSParticleLocal, "Shader/Particle.shader", "VSParticle_local", "vs_5_0" );
IMPLEMENT_MATERIAL_SHADER( VSParticle1, "Shader/Particle1.shader", "VSParticle", "vs_5_0" );
IMPLEMENT_MATERIAL_SHADER( PSParticle1, "Shader/Particle1.shader", "PSParticle", "ps_5_0" );
IMPLEMENT_MATERIAL_SHADER( VSParticleSpark, "Shader/ParticleSpark.shader", "VSParticle", "vs_5_0" );
IMPLEMENT_MATERIAL_SHADER( VSParticleSplash, "Shader/ParticleSplash.shader", "VSParticle", "vs_5_0" );
IMPLEMENT_MATERIAL_SHADER( PSParticleSplashColor, "Shader/ParticleSplash.shader", "PSColor", "ps_5_0" );
IMPLEMENT_MATERIAL_SHADER( PSParticleSplashOcclusion, "Shader/ParticleSplash.shader", "PSOcclusion", "ps_5_0" );
IMPLEMENT_MATERIAL_SHADER( VSParticleFlipbook, "Shader/ParticleFlipbook.shader", "VSParticle", "vs_5_0" );
IMPLEMENT_MATERIAL_SHADER( VSParticleFlipbook1, "Shader/ParticleFlipbook1.shader", "VSParticle", "vs_5_0" );
IMPLEMENT_MATERIAL_SHADER( VSParticleFlipbook1InstData, "Shader/ParticleFlipbook1.shader", "VSParticleInstData", "vs_5_0" );
IMPLEMENT_MATERIAL_SHADER( VSParticleFlipbook1TanVel, "Shader/ParticleFlipbook1.shader", "VSParticle1", "vs_5_0" );
IMPLEMENT_MATERIAL_SHADER( VSParticleFlipbook1TanVel1, "Shader/ParticleFlipbook1.shader", "VSParticle1_1", "vs_5_0" );
IMPLEMENT_MATERIAL_SHADER( VSParticleFlipbook2T, "Shader/ParticleFlipbook1.shader", "VSParticle2", "vs_5_0" );
IMPLEMENT_MATERIAL_SHADER( VSParticleFlipbook2TL, "Shader/ParticleFlipbook1.shader", "VSParticle2L", "vs_5_0" );
IMPLEMENT_MATERIAL_SHADER( GSWrapTex, "Shader/GSWrapTex.shader", "GSMain", "gs_5_0" );

IMPLEMENT_MATERIAL_SHADER( PSWater, "Shader/Water.shader", "PSWater", "ps_5_0" );
IMPLEMENT_MATERIAL_SHADER( PSWaterOneColor, "Shader/Water.shader", "PSWaterOneColor", "ps_5_0" );