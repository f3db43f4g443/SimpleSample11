htx_color_0 = { { 0.67, 0.47, 0.23, 1 }, "typing", 2 }
htx_color_1 = { { 0.65, 0, 0, 1 }, "typing_system_error", 6 }
dtx_color_0 = { { 0.47, 0.05, 0.65, 1 }, "typing", 2 }
dtx_color_1 = { { 0.6, 0.42, 0.1, 1 }, "typing", 2 }
dtx_color_2 = { { 0.2, 0.55, 0.3, 1 }, "typing_system", 2 }
dtx_color_3 = { { 0.65, 0, 0, 1 }, "typing_system_error", 6 }
dtx_color_4 = { { 0.75, 0.8, 0.4, 1 }, "typing_system_warning", 4 }
dtx_color_5 = { { 0.7, 0.7, 0.7, 1 }, "typing_system", 2 }
dtx_color_6 = { { 0.35, 0.35, 0.35, 1 }, "typing", 2 }

LEVEL_GRID_SIZE_X = 24
LEVEL_GRID_SIZE_Y = 32

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

function FEVT( key )
	if EvaluateKeyInt( key ) == 0 then
		SetKeyInt( key, 1 )
		return true
	end
	return false
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


function Day3_Progress( nNode )
 local nProgress = EvaluateKeyInt( "day3_progress" ) + 1
 SetKeyInt( "day3_progress", nProgress )
 GetMasterLevel():RemoveLevelMark( "mark_" .. tostring( nNode ) )
 WaitFor( RunScenarioAndWait( function()
  WaitFor( ScenarioDialogue( 1, "Admin logging in..........OK.", dtx_color_2, 60, 6 ) )
  WaitFor( ScenarioDialogue( 1, "Visiting permission node " .. tostring( nNode ) .. "...", dtx_color_2, 60, 6 ) )
  WaitFor( ScenarioDialogue( 1, "Permission granted.", dtx_color_2, 60 ) )
  WaitFor( ScenarioDialogue( 1, "Permission Tokens: " .. tostring( nProgress ) .. ".", dtx_color_2, 60 ) )
  WaitFor( ScenarioDialogue( 1, tostring( 3 - nProgress ) .. " permission tokens left.", dtx_color_2, -1 ) )
  
  local Func = function( time )
   local p = GetPlayer()
   local src = { ( p:GetPosX() + 1 ) * LEVEL_GRID_SIZE_X, p:GetPosY() * LEVEL_GRID_SIZE_Y }
   local dst1 = { src[1] - 32, src[2] + 256 }
   local dst2 = { src[1] + 32 , src[2] + 256 }
   local dst3 = { src[1], src[2] + 48 }
   local l1 = CreateLighningEft( src, dst1 )
   local l2 = CreateLighningEft( dst1, dst2 )
   local l3 = CreateLighningEft( dst2, dst3 )
   if time > 0 then
    Delay( time )
    l1:SetParentEntity( nil )
    l2:SetParentEntity( nil )
    l3:SetParentEntity( nil )
   end
  end

  if nProgress == 1 then
   WaitFor( ScenarioDialogue( 0, "I must report back now...", dtx_color_1, -1 ) )
   Delay( 60 )
   WaitFor( ScenarioDialogue( 1, "Opening admin command line...", dtx_color_2, 60, 6 ) )
   Delay( 60 )
   WaitFor( ScenarioDialogue( 0, "OK. What's the usage then...", dtx_color_1, -1 ) )
   WaitFor( ScenarioDialogue( 0, "> report -h", dtx_color_5, -1, 15 ) )
   WaitFor( ScenarioDialogue( 1, "HELP CONTENT - report ( COMMAND )", dtx_color_5, -1 ) )
   WaitFor( ScenarioDialogue( 1, "COMMAND TYPE: LOG, IMPORTANT", dtx_color_5, -1 ) )
   WaitFor( ScenarioDialogue( 1, "REQUIRED PERMISSION: ADMIN", dtx_color_5, -1 ) )
   WaitFor( ScenarioDialogue( 1, "COMMAND DESCRIPTION:", dtx_color_5, -1 ) )
   WaitFor( ScenarioDialogue( 1, "Report back the experiment progress and", dtx_color_5, -1 ) )
   WaitFor( ScenarioDialogue( 1, "exceptional situations( if any ) to the server.", dtx_color_5, -1 ) )
   WaitFor( ScenarioDialogue( 1, "This command should be executed daily and the", dtx_color_5, -1 ) )
   WaitFor( ScenarioDialogue( 1, "content must follow the specific format. Note that", dtx_color_5, -1 ) )
   WaitFor( ScenarioDialogue( 1, "illegal input will result in undefined behavior.", dtx_color_5, -1 ) )
   WaitFor( ScenarioDialogue( 1, "OPTIONAL PARAMS:", dtx_color_5, -1 ) )
   WaitFor( ScenarioDialogue( 1, "-h     Open this help file", dtx_color_5, -1 ) )
   WaitFor( ScenarioDialogue( 1, "-i     Enter interactive mode", dtx_color_5, -1 ) )
   WaitFor( ScenarioDialogue( 1, "-f     Select a fivneicubz#*(86&%&>@)#", dtx_color_5, 0 ) )
   Func( 30 )
   Delay( 60 )
   WaitFor( ScenarioDialogue( 0, "What the fuck...", dtx_color_1, -1 ) )
   WaitFor( ScenarioDialogue( 0, "sdfsdf oihiuoh sefhihw cxu e u ei efui weif", dtx_color_5, -1, 3 ) )
   Delay( 90 )
   Func( 30 )
   WaitFor( ScenarioDialogue( 0, "....Oh Please not again..." , dtx_color_1, -1 ) )
   WaitFor( ScenarioDialogue( 1, ".........." , dtx_color_0, 60, 6 ) )
   Delay( 30 )
   Func( 30 )
   WaitFor( ScenarioDialogue( 1, ".........." , dtx_color_0, 60, 6 ) )
   WaitFor( ScenarioDialogue( 1, "...TALK TO ME." , dtx_color_0, 60, 6 ) )
   WaitFor( ScenarioDialogue( 1, "STOP RUNNING AWAY." , dtx_color_0, -1, 6 ) )
   WaitFor( ScenarioDialogue( 0, "....No I'm not..." , dtx_color_1, 60 ) )
   WaitFor( ScenarioDialogue( 0, "Listen, you have to trust me..." , dtx_color_1, 60 ) )
   WaitFor( ScenarioDialogue( 0, "We are both in trouble..." , dtx_color_1, 60 ) )
   WaitFor( ScenarioDialogue( 0, "I am trying to fix this..." , dtx_color_1, -1 ) )
   WaitFor( ScenarioDialogue( 1, ".........." , dtx_color_0, 60, 6 ) )
   Delay( 30 )
   Func( 30 )
   WaitFor( ScenarioDialogue( 1, "...I WANT FOOD." , dtx_color_0, -1 ) )
   WaitFor( ScenarioDialogue( 1, "...NOW." , dtx_color_0, -1 ) )
  elseif nProgress == 2 then
   Delay( 60 )
   WaitFor( ScenarioDialogue( 0, "> repor", dtx_color_5, 1, 15 ) )
   Func( 30 )
   WaitFor( ScenarioDialogue( 0, "....Oh stop please..." , dtx_color_1, 60 ) )
   WaitFor( ScenarioDialogue( 0, "....I have not been doing anything yet..." , dtx_color_1, -1 ) )
   WaitFor( ScenarioDialogue( 1, "...FOOD." , dtx_color_0, 60 ) )
   WaitFor( ScenarioDialogue( 1, "...NOW." , dtx_color_0, -1 ) )
   WaitFor( ScenarioDialogue( 0, "Please let me finish my job..." , dtx_color_1, 60 ) )
   WaitFor( ScenarioDialogue( 0, "I'll restore the food supply then..." , dtx_color_1, -1 ) )
   Func( 30 )
   WaitFor( ScenarioDialogue( 1, "...NOW." , dtx_color_0, -1 ) )
   WaitFor( ScenarioDialogue( 0, ".........." , dtx_color_1, 60, 6 ) )
   WaitFor( ScenarioDialogue( 0, "This will not help..." , dtx_color_1, 60 ) )
   WaitFor( ScenarioDialogue( 0, "I can't give you anything before it gets fixed..." , dtx_color_1, 60 ) )
  else
   WaitFor( ScenarioDialogue( 1, "All Permissions granted.", dtx_color_2, 60 ) )
   WaitFor( ScenarioDialogue( 1, "Please return to the initial terminal", dtx_color_2, 60 ) )
   WaitFor( ScenarioDialogue( 1, "to continue the operation.", dtx_color_2, -1 ) )
   Delay( 60 )
   Func( 30 )
   WaitFor( ScenarioDialogue( 0, ".........." , dtx_color_1, 60, 6 ) )
   WaitFor( ScenarioDialogue( 0, "Didn't you had enough you bloody crazy mother..." , dtx_color_1, -1 ) )
   Func( 30 )
   WaitFor( ScenarioDialogue( 1, "...NOW." , dtx_color_0, -1 ) )
   WaitFor( ScenarioDialogue( 0, "...OK ok. I know you are hungry..." , dtx_color_1, 60 ) )
   WaitFor( ScenarioDialogue( 0, "...And weak..." , dtx_color_1, 60 ) )
   WaitFor( ScenarioDialogue( 0, "...And fucking mad..." , dtx_color_1, 60 ) )
   WaitFor( ScenarioDialogue( 0, "...Just please wait a minute..." , dtx_color_1, 60 ) )
   WaitFor( ScenarioDialogue( 0, "...and I swear I'll send you food soon..." , dtx_color_1, 60 ) )
   WaitFor( ScenarioDialogue( 0, "...Just let me handle my ex...ception first." , dtx_color_1, 60 ) )
   WaitFor( ScenarioDialogue( 0, "...It's nearly done. Just let me get this done OK?" , dtx_color_1, -1 ) )
  end
 end ) )
end
