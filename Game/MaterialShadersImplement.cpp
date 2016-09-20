#include "stdafx.h"
#include "Material.h"

void Game_ShaderImplement_Dummy_Effect();
void Game_ShaderImplement_Dummy_MainUI();
void Game_ShaderImplement_Dummy_PostEffects();
void Game_ShaderImplement_Dummy()
{
	Engine_ShaderImplement_Dummy();
	Game_ShaderImplement_Dummy_Effect();
	Game_ShaderImplement_Dummy_MainUI();
	Game_ShaderImplement_Dummy_PostEffects();
}

IMPLEMENT_MATERIAL_SHADER( PSBloodStain, "Shader/Blood.shader", "PSBloodStain", "ps_5_0" );
IMPLEMENT_MATERIAL_SHADER( VSHpBar, "Shader/HpBar.shader", "VSHpBar", "vs_5_0" );
IMPLEMENT_MATERIAL_SHADER( PSHpBar, "Shader/HpBar.shader", "PSHpBar", "ps_5_0" );
IMPLEMENT_MATERIAL_SHADER( PSHpBarBorder, "Shader/HpBar.shader", "PSHpBarBorder", "ps_5_0" );
IMPLEMENT_MATERIAL_SHADER( VSParticleCommon, "Shader/ParticleCommon.shader", "VSParticle", "vs_5_0" );
IMPLEMENT_MATERIAL_SHADER( VSParticle, "Shader/Particle.shader", "VSParticle", "vs_5_0" );
IMPLEMENT_MATERIAL_SHADER( VSParticleLocal, "Shader/Particle.shader", "VSParticle_local", "vs_5_0" );
IMPLEMENT_MATERIAL_SHADER( VSParticle1, "Shader/Particle1.shader", "VSParticle", "vs_5_0" );
IMPLEMENT_MATERIAL_SHADER( PSParticle1, "Shader/Particle1.shader", "PSParticle", "ps_5_0" );
IMPLEMENT_MATERIAL_SHADER( PSPlayerCrosshair, "Shader/PlayerCrosshair.shader", "PSMain", "ps_5_0" );
IMPLEMENT_MATERIAL_SHADER( VSHUDCircle, "Shader/HUDCircle.shader", "VSMain", "vs_5_0" );
IMPLEMENT_MATERIAL_SHADER( PSHUDCircle, "Shader/HUDCircle.shader", "PSMain", "ps_5_0" );
IMPLEMENT_MATERIAL_SHADER( PSHUDCirclePercent, "Shader/HUDCircle.shader", "PSMainPercent", "ps_5_0" );
IMPLEMENT_MATERIAL_SHADER( VSParticleSplash, "Shader/ParticleSplash.shader", "VSParticle", "vs_5_0" );
IMPLEMENT_MATERIAL_SHADER( PSParticleSplashColor, "Shader/ParticleSplash.shader", "PSColor", "ps_5_0" );
IMPLEMENT_MATERIAL_SHADER( PSParticleSplashOcclusion, "Shader/ParticleSplash.shader", "PSOcclusion", "ps_5_0" );
IMPLEMENT_MATERIAL_SHADER( PSFootprintUpdate, "Shader/Footprint.shader", "PSUpdate", "ps_5_0" );
IMPLEMENT_MATERIAL_SHADER( PSFootprintAlphaToOcclusion, "Shader/Footprint.shader", "PSPrint_AlphaToOcclusion", "ps_5_0" );
IMPLEMENT_MATERIAL_SHADER( PSFootprintLiquidColor, "Shader/FootprintLiquid.shader", "PSColor", "ps_5_0" );
IMPLEMENT_MATERIAL_SHADER( PSFootprintLiquidOcclusion, "Shader/FootprintLiquid.shader", "PSOcclusion", "ps_5_0" );
IMPLEMENT_MATERIAL_SHADER( VSFootprintLiquidParticle, "Shader/FootprintLiquid.shader", "VSParticle", "vs_5_0" );
IMPLEMENT_MATERIAL_SHADER( VSSpiderWebSilkParticle, "Shader/SpiderWebSilk.shader", "VSParticle", "vs_5_0" );
IMPLEMENT_MATERIAL_SHADER( VSLaserParticle, "Shader/Laser.shader", "VSParticle", "vs_5_0" );
IMPLEMENT_MATERIAL_SHADER( PSLaserParticle, "Shader/Laser.shader", "PSParticle", "ps_5_0" );
IMPLEMENT_MATERIAL_SHADER( PSLaserParticleOcclusion, "Shader/Laser.shader", "PSParticleOcclusion", "ps_5_0" );
IMPLEMENT_MATERIAL_SHADER( GSWrapTex, "Shader/GSWrapTex.shader", "GSMain", "gs_5_0" );

IMPLEMENT_MATERIAL_SHADER( PSBlisterColor, "Shader/Slime.shader", "PSBlisterColor", "ps_5_0" );
IMPLEMENT_MATERIAL_SHADER( PSBlisterOcclusion, "Shader/Slime.shader", "PSBlisterOcclusion", "ps_5_0" );

IMPLEMENT_MATERIAL_SHADER( PSTutorialGame, "Shader/TutorialGame.shader", "PSMain", "ps_5_0" );
IMPLEMENT_MATERIAL_SHADER( PSScanLineBase, "Shader/TutorialGame.shader", "PSScanLineBase", "ps_5_0" );
IMPLEMENT_MATERIAL_SHADER( PSScanLineBase1, "Shader/TutorialGame.shader", "PSScanLineBase1", "ps_5_0" );
IMPLEMENT_MATERIAL_SHADER( PSScanLine, "Shader/TutorialGame.shader", "PSScanLine", "ps_5_0" );
IMPLEMENT_MATERIAL_SHADER( PSScanLine1, "Shader/TutorialGame.shader", "PSScanLine1", "ps_5_0" );
IMPLEMENT_MATERIAL_SHADER( PSTutorialGameFlash, "Shader/TutorialGame.shader", "PSFlash", "ps_5_0" );
IMPLEMENT_MATERIAL_SHADER( PSScrCoordMasked, "Shader/TutorialGame.shader", "PSScrCoordMasked", "ps_5_0" );
IMPLEMENT_MATERIAL_SHADER( PSTutorialGameHREffect, "Shader/TutorialGame.shader", "PSHREffect", "ps_5_0" );
IMPLEMENT_MATERIAL_SHADER( VSParticleGlitch, "Shader/TutorialGame_Particle.shader", "VSParticleGlitch", "vs_5_0" );