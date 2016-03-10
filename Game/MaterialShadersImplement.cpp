#include "stdafx.h"
#include "Material.h"

IMPLEMENT_MATERIAL_SHADER( PSBloodStain, "Shader/Blood.shader", "PSBloodStain", "ps_5_0" );
IMPLEMENT_MATERIAL_SHADER( VSHpBar, "Shader/HpBar.shader", "VSHpBar", "vs_5_0" );
IMPLEMENT_MATERIAL_SHADER( PSHpBar, "Shader/HpBar.shader", "PSHpBar", "ps_5_0" );
IMPLEMENT_MATERIAL_SHADER( PSHpBarBorder, "Shader/HpBar.shader", "PSHpBarBorder", "ps_5_0" );
IMPLEMENT_MATERIAL_SHADER( VSParticle, "Shader/Particle.shader", "VSParticle", "vs_5_0" );
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