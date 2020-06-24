htx_color_0 = { { 0.67, 0.47, 0.23, 1 }, "typing", 2 }
htx_color_1 = { { 0.65, 0, 0, 1 }, "typing_system_error", 6 }
dtx_color_0 = { { 0.47, 0.05, 0.65, 1 }, "typing", 2 }
dtx_color_1 = { { 0.6, 0.42, 0.1, 1 }, "typing", 2 }
dtx_color_2 = { { 0.2, 0.55, 0.3, 1 }, "typing_system", 2 }
dtx_color_3 = { { 0.65, 0, 0, 1 }, "typing_system_error", 6 }
dtx_color_4 = { { 0.75, 0.8, 0.4, 1 }, "typing_system_warning", 4 }
dtx_color_5 = { { 0.7, 0.7, 0.7, 1 }, "typing", 2 }
dtx_color_6 = { { 0.35, 0.35, 0.35, 1 }, "typing", 2 }

function Delay( n )
	for i = 1, n, 1 do
		coroutine.yield()
	end
end

function WaitFor( ... )
	local arg = {...}
	for k, v in pairs( arg ) do
		if type( v ) == "function" then
			while not v() do
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

function ScenarioDialogue( n, sz, type, nFinishDelay, nSpeed )
	nSpeed = nSpeed or 1
	GetMasterLevel():GetMainUI():ScenarioText( n, sz, type[1], nFinishDelay, nSpeed, type[2], type[3] )
	return function()
		return GetMasterLevel():GetMainUI():IsScenarioTextFinished()
	end
end

function HeadText( sz, type, time )
	time = time or 0
	color = color or htx_color_0
	GetMasterLevel():GetMainUI():HeadText( sz, type[1], time, type[2], type[3] )
end

function EvaluateKeyInt( sz )
	return GetMasterLevel():EvaluateKeyInt( sz )
end

function Lock( sz )
	return GetMasterLevel():EvaluateKeyInt( sz ) == 0
end

function SetKeyInt( sz, value )
	GetMasterLevel():SetKeyInt( sz, value )
end

function ClearKeys()
	GetMasterLevel():ClearKeys()
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

function TransferTo( sz, x, y, dir )
	GetMasterLevel():ScriptTransferTo( sz, x, y, dir )
end