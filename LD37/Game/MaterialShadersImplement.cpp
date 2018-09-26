#include "stdafx.h"
#include "Material.h"

void Game_ShaderImplement_Dummy_Level();
void Game_ShaderImplement_Dummy_Effect();
void Game_ShaderImplement_Dummy_MainUI();
void Game_ShaderImplement_Dummy_PostEffects();
void Game_ShaderImplement_Dummy_Splash();
void Game_ShaderImplement_Dummy()
{
	Engine_ShaderImplement_Dummy();
	Game_ShaderImplement_Dummy_Level();
	Game_ShaderImplement_Dummy_Effect();
	Game_ShaderImplement_Dummy_MainUI();
	Game_ShaderImplement_Dummy_PostEffects();
	Game_ShaderImplement_Dummy_Splash();
}

template<uint32 InstCount>
class CMyDefault2DVertexShaderExtraInstData : public CGlobalShader
{
protected:
	virtual void SetMacros( SShaderMacroDef& macros ) override
	{
		char szBuf[32];
		sprintf( szBuf, "%d", InstCount );
		macros.Add( "EXTRA_INST_DATA", szBuf );
	}
};

IMPLEMENT_MATERIAL_SHADER( Default2DWithOrigTexVertexShader, "Shader/Default2DWithOrigTex.shader", "VSDefault", "vs_5_0" );
IMPLEMENT_MATERIAL_SHADER_WITH_CLASS( Default2DWithOrigTexVertexShader1, CMyDefault2DVertexShaderExtraInstData<1>, "Shader/Default2DWithOrigTex.shader", "VSDefaultExtraInstData", "vs_5_0" );

IMPLEMENT_MATERIAL_SHADER( VSBlockRTLayer, "Shader/Blocks.shader", "VSBlockRTLayer", "vs_5_0" );
IMPLEMENT_MATERIAL_SHADER( PSBlockRTLayer, "Shader/Blocks.shader", "PSBlockRTLayer", "ps_5_0" );
IMPLEMENT_MATERIAL_SHADER( PSBlockRTLayer1, "Shader/Blocks.shader", "PSBlockRTLayer1", "ps_5_0" );

IMPLEMENT_MATERIAL_SHADER( PSTwoColorLerp, "Shader/Effect.shader", "PSTwoColorLerp", "ps_5_0" );
IMPLEMENT_MATERIAL_SHADER( PSSingleColorEmissionClearColorClip, "Shader/Effect.shader", "PSSingleColorEmissionClearColorClip", "ps_5_0" );
IMPLEMENT_MATERIAL_SHADER( PSSingleColorEmissionClearColorNoClip, "Shader/Effect.shader", "PSSingleColorEmissionClearColorNoClip", "ps_5_0" );
IMPLEMENT_MATERIAL_SHADER( PSEmissionClearColorClip, "Shader/Effect.shader", "PSEmissionClearColorClip", "ps_5_0" );
IMPLEMENT_MATERIAL_SHADER( PSEmissionClearColorNoClip, "Shader/Effect.shader", "PSEmissionClearColorNoClip", "ps_5_0" );
IMPLEMENT_MATERIAL_SHADER( PSEmissionClearColorInstData, "Shader/Effect.shader", "PSEmissionClearColorInstData", "ps_5_0" );
IMPLEMENT_MATERIAL_SHADER( PSEmissionEffect, "Shader/Effect.shader", "PSEmissionEffect", "ps_5_0" );
IMPLEMENT_MATERIAL_SHADER( PSEmissionEffect1, "Shader/Effect.shader", "PSEmissionEffect1", "ps_5_0" );
IMPLEMENT_MATERIAL_SHADER( PSParticleDissolveColor, "Shader/Effect.shader", "PSParticleDissolveColor", "ps_5_0" );
IMPLEMENT_MATERIAL_SHADER( PSParticleDissolveOcc, "Shader/Effect.shader", "PSParticleDissolveOcc", "ps_5_0" );
IMPLEMENT_MATERIAL_SHADER( PSEmiRadialFade, "Shader/Effect.shader", "PSEmiRadialFade", "ps_5_0" );

IMPLEMENT_MATERIAL_SHADER( PSBulletEffect, "Shader/Bullet.shader", "PSBulletEffect", "ps_5_0" );

IMPLEMENT_MATERIAL_SHADER( PSBloodStain, "Shader/Blood.shader", "PSBloodStain", "ps_5_0" );
IMPLEMENT_MATERIAL_SHADER( PSHpBar, "Shader/HpBar.shader", "PSHpBar", "ps_5_0" );
IMPLEMENT_MATERIAL_SHADER( PSFaceSelectArea, "Shader/SelectArea.shader", "PSFaceSelectArea", "ps_5_0" );
IMPLEMENT_MATERIAL_SHADER( VSParticleCommon, "Shader/ParticleCommon.shader", "VSParticle", "vs_5_0" );
IMPLEMENT_MATERIAL_SHADER( VSParticle, "Shader/Particle.shader", "VSParticle", "vs_5_0" );
IMPLEMENT_MATERIAL_SHADER( VSParticleLocal, "Shader/Particle.shader", "VSParticle_local", "vs_5_0" );
IMPLEMENT_MATERIAL_SHADER( VSParticle1, "Shader/Particle1.shader", "VSParticle", "vs_5_0" );
IMPLEMENT_MATERIAL_SHADER( VSParticle1_1, "Shader/Particle1.shader", "VSParticle1", "vs_5_0" );
IMPLEMENT_MATERIAL_SHADER( VSParticle1_2, "Shader/Particle1.shader", "VSParticle2", "vs_5_0" );
IMPLEMENT_MATERIAL_SHADER( PSParticle1, "Shader/Particle1.shader", "PSParticle", "ps_5_0" );
IMPLEMENT_MATERIAL_SHADER( PSParticleColorMapColor, "Shader/Particle1.shader", "PSParticleColorMapColor", "ps_5_0" );
IMPLEMENT_MATERIAL_SHADER( PSParticleColorMapOcclusion, "Shader/Particle1.shader", "PSParticleColorMapOcclusion", "ps_5_0" );
IMPLEMENT_MATERIAL_SHADER( VSHUDCircle, "Shader/HUDCircle.shader", "VSMain", "vs_5_0" );
IMPLEMENT_MATERIAL_SHADER( PSHUDCircle, "Shader/HUDCircle.shader", "PSMain", "ps_5_0" );
IMPLEMENT_MATERIAL_SHADER( PSHUDCirclePercent, "Shader/HUDCircle.shader", "PSMainPercent", "ps_5_0" );
IMPLEMENT_MATERIAL_SHADER( VSParticleFlipbook, "Shader/ParticleFlipbook.shader", "VSParticle", "vs_5_0" );
IMPLEMENT_MATERIAL_SHADER( VSParticleFlipbook1, "Shader/ParticleFlipbook1.shader", "VSParticle", "vs_5_0" );
IMPLEMENT_MATERIAL_SHADER( VSParticleFlipbook1InstData, "Shader/ParticleFlipbook1.shader", "VSParticleInstData", "vs_5_0" );
IMPLEMENT_MATERIAL_SHADER( VSParticleFlipbook1TanVel, "Shader/ParticleFlipbook1.shader", "VSParticle1", "vs_5_0" );
IMPLEMENT_MATERIAL_SHADER( VSParticleFlipbook1TanVel1, "Shader/ParticleFlipbook1.shader", "VSParticle1_1", "vs_5_0" );
IMPLEMENT_MATERIAL_SHADER( VSParticleFlipbook1Splash, "Shader/ParticleFlipbookSplash.shader", "VSParticle1Splash", "vs_5_0" );
IMPLEMENT_MATERIAL_SHADER( VSParticleFlipbook2T, "Shader/ParticleFlipbook1.shader", "VSParticle2", "vs_5_0" );
IMPLEMENT_MATERIAL_SHADER( VSParticleFlipbook2TL, "Shader/ParticleFlipbook1.shader", "VSParticle2L", "vs_5_0" );
IMPLEMENT_MATERIAL_SHADER( PSFootprintUpdate, "Shader/Footprint.shader", "PSUpdate", "ps_5_0" );
IMPLEMENT_MATERIAL_SHADER( PSFootprintAlphaToOcclusion, "Shader/Footprint.shader", "PSPrint_AlphaToOcclusion", "ps_5_0" );
IMPLEMENT_MATERIAL_SHADER( PSFootprintLiquidColor, "Shader/FootprintLiquid.shader", "PSColor", "ps_5_0" );
IMPLEMENT_MATERIAL_SHADER( PSFootprintLiquidOcclusion, "Shader/FootprintLiquid.shader", "PSOcclusion", "ps_5_0" );
IMPLEMENT_MATERIAL_SHADER( VSFootprintLiquidParticle, "Shader/FootprintLiquid.shader", "VSParticle", "vs_5_0" );
IMPLEMENT_MATERIAL_SHADER( VSThruster, "Shader/Beams.shader", "VSThruster", "vs_5_0" );
IMPLEMENT_MATERIAL_SHADER( PSThruster, "Shader/Beams.shader", "PSThruster", "ps_5_0" );
IMPLEMENT_MATERIAL_SHADER( VSThruster1, "Shader/Beams.shader", "VSThruster1", "vs_5_0" );
IMPLEMENT_MATERIAL_SHADER( PSThruster1Color, "Shader/Beams.shader", "PSThruster1Color", "ps_5_0" );
IMPLEMENT_MATERIAL_SHADER( PSThruster1Occ, "Shader/Beams.shader", "PSThruster1Occ", "ps_5_0" );
IMPLEMENT_MATERIAL_SHADER( VSLaserParticle, "Shader/Laser.shader", "VSParticle", "vs_5_0" );
IMPLEMENT_MATERIAL_SHADER( VSLaserParticle1, "Shader/Laser.shader", "VSParticle1", "vs_5_0" );
IMPLEMENT_MATERIAL_SHADER( PSLaserParticle, "Shader/Laser.shader", "PSParticle", "ps_5_0" );
IMPLEMENT_MATERIAL_SHADER( PSLaserParticleOcclusion, "Shader/Laser.shader", "PSParticleOcclusion", "ps_5_0" );
IMPLEMENT_MATERIAL_SHADER( VSParticleRandomTex, "Shader/ParticleRandomTex.shader", "VSParticle", "vs_5_0" );
IMPLEMENT_MATERIAL_SHADER( PSBloodWave, "Shader/ParticleRandomTex.shader", "PSMask", "ps_5_0" );
IMPLEMENT_MATERIAL_SHADER( PSBloodWaveOcc, "Shader/ParticleRandomTex.shader", "PSMaskOcclusion", "ps_5_0" );
IMPLEMENT_MATERIAL_SHADER( GSWrapTex, "Shader/GSWrapTex.shader", "GSMain", "gs_5_0" );
IMPLEMENT_MATERIAL_SHADER( GSWrapTex1, "Shader/GSWrapTex.shader", "GSMain1", "gs_5_0" );
IMPLEMENT_MATERIAL_SHADER( GSWrapTex2, "Shader/GSWrapTex.shader", "GSMain2", "gs_5_0" );

IMPLEMENT_MATERIAL_SHADER( PSDecoratorBlendMul, "Shader/Decorator.shader", "PSBlendMul", "ps_5_0" );
IMPLEMENT_MATERIAL_SHADER( PSDecoratorBlendVividLight, "Shader/Decorator.shader", "PSBlendVividLight", "ps_5_0" );

IMPLEMENT_MATERIAL_SHADER( PSTutorialGame, "Shader/TutorialGame.shader", "PSMain", "ps_5_0" );
IMPLEMENT_MATERIAL_SHADER( PSScanLineBase, "Shader/TutorialGame.shader", "PSScanLineBase", "ps_5_0" );
IMPLEMENT_MATERIAL_SHADER( PSScanLineBase1, "Shader/TutorialGame.shader", "PSScanLineBase1", "ps_5_0" );
IMPLEMENT_MATERIAL_SHADER( PSScanLine, "Shader/TutorialGame.shader", "PSScanLine", "ps_5_0" );
IMPLEMENT_MATERIAL_SHADER( PSScanLine1, "Shader/TutorialGame.shader", "PSScanLine1", "ps_5_0" );
IMPLEMENT_MATERIAL_SHADER( PSTutorialGameFlash, "Shader/TutorialGame.shader", "PSFlash", "ps_5_0" );
IMPLEMENT_MATERIAL_SHADER( PSScrCoordMasked, "Shader/TutorialGame.shader", "PSScrCoordMasked", "ps_5_0" );
IMPLEMENT_MATERIAL_SHADER( PSTutorialGameHREffect, "Shader/TutorialGame.shader", "PSHREffect", "ps_5_0" );
IMPLEMENT_MATERIAL_SHADER( VSParticleGlitch, "Shader/TutorialGame_Particle.shader", "VSParticleGlitch", "vs_5_0" );

IMPLEMENT_MATERIAL_SHADER( PSWater, "Shader/Water.shader", "PSWater", "ps_5_0" );
IMPLEMENT_MATERIAL_SHADER( PSWaterOneColor, "Shader/Water.shader", "PSWaterOneColor", "ps_5_0" );

IMPLEMENT_MATERIAL_SHADER( PSDistortionMaskColor, "Shader/Distortion.shader", "PSDistortionMaskColor", "ps_5_0" );
IMPLEMENT_MATERIAL_SHADER( PSDistortionMaskOcc, "Shader/Distortion.shader", "PSDistortionMaskOcc", "ps_5_0" );