g_test = true

VK_ESCAPE = 27
VK_RETURN = 13
VK_LEFT = 37
VK_UP = 38
VK_RIGHT = 39
VK_DOWN = 40

NAME_LAWYER = "Harrison"

htx_color_0 = { { 0.67, 0.47, 0.23, 1 }, "typing", 2 }
htx_color_x = { { 0.47, 0.05, 0.65, 1 }, "typing", 2 }
htx_color_1 = { { 0.65, 0, 0, 1 }, "typing_system_error", 6 }
htx_color_2 = { { 0.75, 0.8, 0.4, 1 }, "typing_system_warning", 4 }
htx_color_sys = { { 0.2, 0.55, 0.3, 1 }, "typing_system", 2 }
htx_color_5 = { { 0.7, 0.7, 0.7, 1 }, "typing", 2 }
htx_color_6 = { { 0.35, 0.35, 0.35, 1 }, "typing", 2 }
htx_color_6a = { { 0.65, 0.55, 0.55, 1 }, "typing", 2 }
htx_color_6b = { { 0.55, 0.55, 0.65, 1 }, "typing", 2 }
htx_color_h = { { 1, 0.4, 0.35, 1 }, "typing", 2 }

dtx_color_0 = { { 0.47, 0.05, 0.65, 1 }, "typing", 2 }
dtx_color_1 = { { 0.6, 0.42, 0.1, 1 }, "typing", 2 }
dtx_color_2 = { { 0.2, 0.55, 0.3, 1 }, "typing_system", 2 }
dtx_color_3 = { { 0.65, 0, 0, 1 }, "typing_system_error", 6 }
dtx_color_4 = { { 0.75, 0.8, 0.4, 1 }, "typing_system_warning", 4 }
dtx_color_5 = { { 0.7, 0.7, 0.7, 1 }, "typing_system", 2 }
dtx_color_6 = { { 0.35, 0.35, 0.35, 1 }, "typing", 2 }
dtx_color_6a = { { 0.45, 0.35, 0.35, 1 }, "typing", 2 }
dtx_color_6b = { { 0.35, 0.35, 0.45, 1 }, "typing", 2 }
dtx_color_h = { { 1, 0.4, 0.35, 1 }, "typing", 2 }

LEVEL_GRID_SIZE_X = 24
LEVEL_GRID_SIZE_Y = 32

function Delay( n, ... )
	if n == 1 then
		return coroutine.yield( ... )
	end
	coroutine.yield( ... )
	for i = 2, n - 1, 1 do
		coroutine.yield()
	end
	return coroutine.yield()
end

function WaitFor( ... )
	local arg = {...}
	local n = 0
	for k, v in pairs( arg ) do
		if type( v ) == "function" then
			while not v() do
				n = n + 1
				coroutine.yield()
			end
		elseif type( v ) == "number" then
			while n < v do
				n = n + 1
				coroutine.yield()
			end
		end
	end
end

function PlayStateAndWait( pawn, szName )
	pawn:PlayState( szName )
	return function()
		return pawn:GetCurStateName() ~= szName
	end
end

function RunScenarioAndWait( ... )
	RunScenario( ... )
	return function()
		return not GetMasterLevel():IsScenario()
	end
end

function ScenarioDialogue( n, sz, type, nFinishDelay, nSpeed )
	nSpeed = nSpeed or 1
	GetMasterLevel():GetMainUI():ScenarioText( n, sz, type[1], nFinishDelay, nSpeed, type[2], type[3] )
	return function()
		return GetMasterLevel():GetMainUI():IsScenarioTextFinished()
	end
end

function HeadText( sz, type, time )
	time = time or 0
	type = type or htx_color_0
	GetMasterLevel():GetMainUI():HeadText( sz, type[1], time, type[2], type[3] )
end

function EvaluateKeyInt( sz )
	return GetMasterLevel():EvaluateKeyInt( sz )
end

function EvaluateKeyString( sz )
	return GetMasterLevel():EvaluateKeyString( sz )
end

function Lock( sz )
	return GetMasterLevel():EvaluateKeyInt( sz ) == 0
end

function SetKeyInt( sz, value )
	GetMasterLevel():SetKeyInt( sz, value )
end

function SetKeyString( sz, value )
	GetMasterLevel():SetKeyString( sz, value )
end

function ClearKeys()
	GetMasterLevel():ClearKeys()
end

function FEVT( key )
	if EvaluateKeyInt( key ) == 0 then
		SetKeyInt( key, 1 )
		return true
	end
	return false
end

function SetLevelIgnoreGlobalClearKeys( sz, b )
	GetMasterLevel():SetLevelIgnoreGlobalClearKeys( sz, b )
end

function CurDay()
	return EvaluateKeyInt( "day" )
end

function CurTime()
	return EvaluateKeyInt( "time" )
end

function SetCurTime( i )
	SetKeyInt( "time", i )
end

function TransferTo( sz, x, y, dir, type )
	GetMasterLevel():ScriptTransferTo( sz, x, y, dir, type )
end

function CreateLighningEft( src, dst, fStrength, fTurbulence )
	return CreateLighningEft_Script( src, dst, fStrength or 1, fTurbulence or 2 )
end

function OnRefreshMainUI()
	RefreshAllLabels()
end

RunLuaFile( "scripts/player.lua" )
RunLuaFile( "scripts/labels.lua" )
RunLuaFile( "scripts/docs.lua" )
RunLuaFile( "scripts/ai.lua" )
RunLuaFile( "scripts/interaction.lua" )
RunLuaFile( "scripts/data_misc.lua" )
RunLuaFile( "scripts/misc.lua" )
--[[

]]