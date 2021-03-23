function Scenario_SelfRoom_3()
 local bed = GetCurLevel():GetPawnByName( "bed_wake_up" )
 WaitFor( ScenarioDialogue( 1, "Good morning, " .. GetPlayerName(), dtx_color_0, 60 ) )
 WaitFor( ScenarioDialogue( 1, "It is 8:00 now.", dtx_color_0, 60 ) )
 WaitFor( ScenarioDialogue( 1, "Please get up.", dtx_color_0, -1 ) )
 WaitFor( ScenarioDialogue( 0, ".............", dtx_color_1, -1, 6 ) )

 ScenarioDialogue( 1, "GET UP.", dtx_color_0, -1 )
 local proj = GetCurLevel():FindChildEntity("proj")
 local src = proj:GetProjSrc()
 local dst = { 240, 48 }
 local l = CreateLighningEft( src, dst )
 GetMasterLevel():InterferenceStripEffect( 1, 2 )
 Delay( 60 )
 GetMasterLevel():BlackOut( 20, 10 )
 GetMasterLevel():InterferenceStripEffect( 0, 0 )
 l:SetParentEntity( nil )
 bed:PlayState( "stand" )
 local player = GetPlayer()
 player:SetForceHide( false )
 player:PlayState( "break" )
 Delay( 45 )
 Delay( 120 )

 ScenarioDialogue( 1, "AND SPEAK TO ME.", dtx_color_0, -1 )
 dst = { 216, 80 }
 local l = CreateLighningEft( src, dst )
 GetMasterLevel():InterferenceStripEffect( 1, 2 )
 Delay( 60 )
 GetMasterLevel():BlackOut( 20, 15 )
 GetMasterLevel():InterferenceStripEffect( 0, 0 )
 player:PlayState( "break" )
 Delay( 90 )
 l:SetParentEntity( nil )
 Delay( 120 )

 SetKeyInt( "$passed", 1 )
 WaitFor( ScenarioDialogue( 0, ".............", dtx_color_1, -1, 6 ) )
 Delay( 60 )
end

function Scenario_SelfRoom_4_0()
 local proj = GetCurLevel():FindChildEntity("proj")
 local src = proj:GetProjSrc()
 local dst = { 240, 48 }
 local l = CreateLighningEft( src, dst )
 GetMasterLevel():InterferenceStripEffect( 1, 1 )
 Delay( 90 )
 GetMasterLevel():InterferenceStripEffect( 0, 0 )
 l:SetParentEntity( nil )
 local bed = GetCurLevel():GetPawnByName( "bed_wake_up" )
 WaitFor( ScenarioDialogue( 1, ".............", dtx_color_0, -1, 6 ), PlayStateAndWait( bed, "wake" ) )
 SetKeyInt( "$passed", 1 )

 WaitFor( ScenarioDialogue( 0, "You look still not so good.", dtx_color_1, 60 ) )
 WaitFor( ScenarioDialogue( 0, "Need any help?", dtx_color_1, -1 ) )

 dst = { 216, 80 }
 l = CreateLighningEft( src, dst )
 GetMasterLevel():InterferenceStripEffect( 1, 1 )
 ScenarioDialogue( 1, ".............", dtx_color_0, -1 )
 Delay( 60 )
 Delay( 45 )
 bed:PlayState( "stand" )
 local player = GetPlayer()
 player:SetForceHide( false )
 player:PlayState( "break" )
 Delay( 45 )
 GetMasterLevel():InterferenceStripEffect( 0, 0 )
 l:SetParentEntity( nil )
 Delay( 60 )
end

function Scenario_SelfRoom_4_1()
 local proj = GetCurLevel():FindChildEntity("proj")
 local src = proj:GetProjSrc()
 local dst = { 240, 48 }
 local l = CreateLighningEft( src, dst )
 GetMasterLevel():InterferenceStripEffect( 1, 1 )
 Delay( 90 )
 GetMasterLevel():InterferenceStripEffect( 0, 0 )
 l:SetParentEntity( nil )
 local bed = GetCurLevel():GetPawnByName( "bed_wake_up" )
 WaitFor( ScenarioDialogue( 0, ".............", dtx_color_1, -1, 6 ), PlayStateAndWait( bed, "wake" ) )

 WaitFor( ScenarioDialogue( 1, "Hello my admin. We have something to talk about.", dtx_color_h, 60 ) )
 WaitFor( ScenarioDialogue( 1, "It seems you made a big mess here.", dtx_color_h, -1 ) )
 Delay( 45 )
 bed:PlayState( "stand" )
 local player = GetPlayer()
 player:SetForceHide( false )
 Delay( 60 )
 WaitFor( ScenarioDialogue( 0, "No, it wasn't me, it was...", dtx_color_1, -1, 3 ) )
 WaitFor( ScenarioDialogue( 1, "I'm not asking for an explanation. Don't be that nervous.", dtx_color_h, 200 ) )
 WaitFor( ScenarioDialogue( 1, "We're just talking about the procedure to take.", dtx_color_h, 180 ) )
 WaitFor( ScenarioDialogue( 1, "Such a situation...To be honest, this is not that unexpected.", dtx_color_h, -1 ) )
 WaitFor( ScenarioDialogue( 0, "...No, I am fucking OK...", dtx_color_1, -1, 3 ) )
 WaitFor( ScenarioDialogue( 1, "Such things sometimes DO happen...like your predecessor did.", dtx_color_h, -1 ) )
 WaitFor( ScenarioDialogue( 0, "......No no no......", dtx_color_1, -1, 3 ) )
 WaitFor( ScenarioDialogue( 1, "It is not a mistake or something...Just a common event.", dtx_color_h, 200 ) )
 WaitFor( ScenarioDialogue( 1, "...An event we need some procedures to handle.", dtx_color_h, -1 ) )
 WaitFor( ScenarioDialogue( 0, "......No what......", dtx_color_1, -1, 3 ) )
 WaitFor( ScenarioDialogue( 1, "You don't need to know more. Let's just go to the procedure.", dtx_color_h, 200 ) )
 WaitFor( ScenarioDialogue( 1, "I'm not saying you're mad definitely...You still can prove your sanity.", dtx_color_h, 200 ) )
 WaitFor( ScenarioDialogue( 1, "Please turn on the TV. We've already prepared everything for you.", dtx_color_h, -1 ) )
end

function Scenario_SelfRoom_4_2()
 local proj = GetCurLevel():FindChildEntity("proj")
 local src = proj:GetProjSrc()
 local dst = { 240, 48 }
 local l = CreateLighningEft( src, dst )
 GetMasterLevel():InterferenceStripEffect( 1, 1 )
 Delay( 90 )
 GetMasterLevel():InterferenceStripEffect( 0, 0 )
 l:SetParentEntity( nil )
 local bed = GetCurLevel():GetPawnByName( "bed_wake_up" )
 WaitFor( ScenarioDialogue( 0, ".............", dtx_color_1, -1, 6 ), PlayStateAndWait( bed, "wake" ) )
 Delay( 45 )
 bed:PlayState( "stand" )
 local player = GetPlayer()
 player:SetForceHide( false )
 Delay( 60 )

 WaitFor( ScenarioDialogue( 1, "Hello my admin. What did you go out for?", dtx_color_h, 90 ) )
 WaitFor( ScenarioDialogue( 1, "You know you should never be there at this time.", dtx_color_h, 110 ) )
 WaitFor( ScenarioDialogue( 1, "You are forbidden to contact with anyone.", dtx_color_h, 90 ) )
 WaitFor( ScenarioDialogue( 1, "I hope you haven't forgot it.", dtx_color_h, 90 ) )
 Delay( 60 )
end

function Scenario_SelfRoom_4_3()
 local proj = GetCurLevel():FindChildEntity("proj")
 local src = proj:GetProjSrc()
 local dst = { 240, 48 }
 local l = CreateLighningEft( src, dst )
 GetMasterLevel():InterferenceStripEffect( 1, 1 )
 Delay( 90 )
 GetMasterLevel():InterferenceStripEffect( 0, 0 )
 l:SetParentEntity( nil )
 local bed = GetCurLevel():GetPawnByName( "bed_wake_up" )
 WaitFor( ScenarioDialogue( 0, ".............", dtx_color_1, -1, 6 ), PlayStateAndWait( bed, "wake" ) )
 Delay( 45 )
 bed:PlayState( "stand" )
 local player = GetPlayer()
 player:SetForceHide( false )
 Delay( 60 )

 WaitFor( ScenarioDialogue( 1, "Hello my admin. Maybe I should remind you again?", dtx_color_h, 110 ) )
 WaitFor( ScenarioDialogue( 1, "You need to finish it before lunchtime ends.", dtx_color_h, 90 ) )
 WaitFor( ScenarioDialogue( 1, "Just three words. Four letters each. Does it take so long?", dtx_color_h, 120 ) )
 WaitFor( ScenarioDialogue( 1, "Input them and all is done. Simple and direct.", dtx_color_h, 90 ) )
 WaitFor( ScenarioDialogue( 1, "But instead you've been hanging around for several hours.", dtx_color_h, 120 ) )
 WaitFor( ScenarioDialogue( 1, "What actually happened to you?", dtx_color_h, 90 ) )
 WaitFor( ScenarioDialogue( 0, "..........", dtx_color_1, -1, 6 ) )
 WaitFor( ScenarioDialogue( 0, "You are stumping me.", dtx_color_1, -1 ) )
 WaitFor( ScenarioDialogue( 1, "...What did you say?", dtx_color_h, 90 ) )
 WaitFor( ScenarioDialogue( 1, "So you think that's difficult?", dtx_color_h, 90 ) )
 WaitFor( ScenarioDialogue( 0, "You want me mad.", dtx_color_1, -1 ) )
 
 WaitFor( ScenarioDialogue( 1, "..........", dtx_color_h, 60, 6 ) )
 WaitFor( ScenarioDialogue( 1, "...It seems you didn't figure out the situation.", dtx_color_h, 100 ) )
 WaitFor( ScenarioDialogue( 1, "I am on your side on this thing. It's you who is stumping me.", dtx_color_h, 120 ) )
 WaitFor( ScenarioDialogue( 1, "I'm blocking the lunch until you get it done.", dtx_color_h, 90 ) )
 WaitFor( ScenarioDialogue( 1, "The lunch should have already been finished.", dtx_color_h, 90 ) )
 WaitFor( ScenarioDialogue( 1, "They are waiting outside and I can't wait for you forever.", dtx_color_h, 110 ) )
 WaitFor( ScenarioDialogue( 1, "But you are showing no respect to our job......", dtx_color_h, 90 ) )
 WaitFor( ScenarioDialogue( 0, "Where is " .. NAME_LAWYER .. "?", dtx_color_1, -1, 2 ) )
 WaitFor( ScenarioDialogue( 1, "..........", dtx_color_h, 60, 6 ) )
 WaitFor( ScenarioDialogue( 1, "......Why ask this?", dtx_color_h, -1 ) )
 WaitFor( ScenarioDialogue( 0, "He was my lawyer.", dtx_color_1, -1, 6 ) )
 WaitFor( ScenarioDialogue( 1, "..........", dtx_color_h, 60, 6 ) )
 Delay( 90 )
 
 dst = { 216, 80 }
 local l = CreateLighningEft( src, dst )
 GetMasterLevel():InterferenceStripEffect( 1, 2 )
 Delay( 60 )
 GetMasterLevel():BlackOut( 20, 15 )
 GetMasterLevel():InterferenceStripEffect( 0, 0 )
 player:PlayState( "break" )
 Delay( 90 )
 l:SetParentEntity( nil )
 Delay( 90 )

 WaitFor( ScenarioDialogue( 1, "You are not allowed to ask questions.", dtx_color_h, 90 ) )
 WaitFor( ScenarioDialogue( 1, "Do what you are supposed to do.", dtx_color_h, 60 ) )

 Delay( 60 )
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

  if g_test then
   Delay( 60 )
   WaitFor( ScenarioDialogue( 1, "YOU HAVE COMPLETED THE DEMO", dtx_color_h, -1 ) )
   TransferTo( "data/cutscene/end.pf" )
   return
  end
  
  local Func = function( time, t1 )
   t1 = t1 or 1
   local p = GetPlayer()
   local src = { ( p:GetPosX() + 1 ) * LEVEL_GRID_SIZE_X, p:GetPosY() * LEVEL_GRID_SIZE_Y }
   local dst1 = { src[1] - 32, src[2] + 256 }
   local dst2 = { src[1] + 32 , src[2] + 256 }
   local dst3 = { src[1], src[2] + 48 }
   local l1 = CreateLighningEft( src, dst1 )
   local l2 = CreateLighningEft( dst1, dst2 )
   local l3 = CreateLighningEft( dst2, dst3 )
   GetMasterLevel():InterferenceStripEffect( 1, t1 )
   if time > 0 then
    Delay( time )
    l1:SetParentEntity( nil )
    l2:SetParentEntity( nil )
    l3:SetParentEntity( nil )
    GetMasterLevel():InterferenceStripEffect( 0, 0 )
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


function LibrarySystem_OnUpdate1()
 if g_LibrarySystem_OnAlert_tip_cd then
  g_LibrarySystem_OnAlert_tip_cd = g_LibrarySystem_OnAlert_tip_cd - 1
  if g_LibrarySystem_OnAlert_tip_cd <= 0 then
   g_LibrarySystem_OnAlert_tip_cd = nil
  end
 end

 local player = GetPlayer()
 local posX = player:GetPosX()
 local posY = player:GetPosY()
 local toX = player:GetToX()
 local toY = player:GetToY()
 local bFail = false
 local src, dst
 GetCurLevel():ScriptForEachPawn( function( pwn )
  if not pwn:IsEnemy() and not pwn:HasStateTag( "npc" ) then
   return true
  end
  local posX1 = pwn:GetPosX()
  local posY1 = pwn:GetPosY()
  local dX = math.abs( posX1 - posX )
  local dY = math.abs( posY1 - posY )
  if not player:IsPosHidden() and ( dX == 2 and dY == 0 or dX == 1 and dY == 1 ) then
   src = { ( posX + 1 ) * LEVEL_GRID_SIZE_X, ( posY + 0.5 ) * LEVEL_GRID_SIZE_Y + 32 }
   dst = { ( posX1 + 1 ) * LEVEL_GRID_SIZE_X, ( posY1 + 0.5 ) * LEVEL_GRID_SIZE_Y + 32 }
   bFail = true
   return false
  end
  dX = math.abs( posX1 - toX )
  dY = math.abs( posY1 - toY )
  if not player:IsToHidden() and ( dX == 2 and dY == 0 or dX == 1 and dY == 1 ) then
   src = { ( toX + 1 ) * LEVEL_GRID_SIZE_X, ( toY + 0.5 ) * LEVEL_GRID_SIZE_Y + 32 }
   dst = { ( posX1 + 1 ) * LEVEL_GRID_SIZE_X, ( posY1 + 0.5 ) * LEVEL_GRID_SIZE_Y + 32 }
   bFail = true
   return false
  end

  posX1 = pwn:GetToX()
  posY1 = pwn:GetToY()
  dX = math.abs( posX1 - posX )
  dY = math.abs( posY1 - posY )
  if not player:IsPosHidden() and ( dX == 2 and dY == 0 or dX == 1 and dY == 1 ) then
   src = { ( posX + 1 ) * LEVEL_GRID_SIZE_X, ( posY + 0.5 ) * LEVEL_GRID_SIZE_Y + 32 }
   dst = { ( posX1 + 1 ) * LEVEL_GRID_SIZE_X, ( posY1 + 0.5 ) * LEVEL_GRID_SIZE_Y + 32 }
   bFail = true
   return false
  end
  dX = math.abs( posX1 - toX )
  dY = math.abs( posY1 - toY )
  if not player:IsToHidden() and ( dX == 2 and dY == 0 or dX == 1 and dY == 1 ) then
   src = { ( toX + 1 ) * LEVEL_GRID_SIZE_X, ( toY + 0.5 ) * LEVEL_GRID_SIZE_Y + 32 }
   dst = { ( posX1 + 1 ) * LEVEL_GRID_SIZE_X, ( posY1 + 0.5 ) * LEVEL_GRID_SIZE_Y + 32 }
   bFail = true
   return false
  end

  return true
 end )
 if not bFail then
  return
 end
 
 local tips = { { "KEEP DISTANCE.", htx_color_1 },
  { "TOO CLOSE.", htx_color_1 },
  { "STAY AWAY.", htx_color_1 },
  { "Library rule enforced.", htx_color_2 } }
 local item = tips[RandInt( 0, #tips ) + 1]
 HeadText( item[1], item[2] )
 
 LevelRegisterUpdate1( function()
  Delay( 15 )
  local l = CreateLighningEft( src, dst )
  local proj = GetCurLevel():FindChildEntity( "proj" )
  proj:SetTarget( dst )
  GetMasterLevel():InterferenceStripEffect( 1, 1 )
  GetCurLevel():Fail( 0 )
 end )
end

function LibrarySystem_OnAlert( pawn, x, y )
 local player = GetPlayer()
 local posX = player:GetPosX()
 local posY = player:GetPosY()
 local toX = player:GetToX()
 local toY = player:GetToY()
 local proj = GetCurLevel():FindChildEntity( "proj" )

 if not pawn then
  local dst = { ( toX + 1 ) * LEVEL_GRID_SIZE_X, ( toY + 0.5 ) * LEVEL_GRID_SIZE_Y }
  proj:SetTarget( dst )
  CreateLighningEft( proj:GetProjSrc(), dst )
  HeadText( "Library rule enforced.", htx_color_2 )
  GetMasterLevel():InterferenceStripEffect( 1, 1 )
  GetCurLevel():Fail( 0 )
 end
 
 local dst = { ( x + 1 ) * LEVEL_GRID_SIZE_X, ( y + 0.5 ) * LEVEL_GRID_SIZE_Y }
 proj:SetTarget( dst )
 if x == posX and y == posY or x == toX and y == toY then
  CreateLighningEft( proj:GetProjSrc(), dst )
  local tips = { { "BE QUIET.", htx_color_1 },
   { "Keep quiet or get out of the library.", htx_color_1 },
   { "Noise source detected.", htx_color_2 },
   { "Library rule enforced.", htx_color_2 } }
  local item = tips[RandInt( 0, #tips ) + 1]
  HeadText( item[1], item[2] )
  GetMasterLevel():InterferenceStripEffect( 1, 1 )
  GetCurLevel():Fail( 0 )
 else
  local l = CreateLighningEft( proj:GetProjSrc(), dst, 0.5, 1 )
  LevelRegisterUpdate1( function()
   Delay( 15 )
   l:SetParentEntity( nil )
   Signal( proj, -1 )

   if not g_LibrarySystem_OnAlert_tip_cd then
    g_LibrarySystem_OnAlert_tip_cd = 120
    local tips = { { "WARNING: Noise from unknown source.", htx_color_2 },
    { "WARNING: Unable to identify noise source.", htx_color_2 },
    { "WARNING: An exception occured during noise detection.", htx_color_2 },
    { "ERROR: ASSERT FAILED: IsValid( pSource )", htx_color_1 },
    { "ERROR: Invalid object.", htx_color_1 },
    { "ERROR: Accessing NULL pointer.", htx_color_1 } }
    local item = tips[RandInt( 0, #tips ) + 1]
    HeadText( item[1], item[2], 180 )
   end
  end )
 end
end

function LibrarySystem_TouchAnything()
 local player = GetPlayer()
 local toX = player:GetToX()
 local toY = player:GetToY()
 local dst = { ( toX + 1 ) * LEVEL_GRID_SIZE_X, ( toX + 0.5 ) * LEVEL_GRID_SIZE_Y }
 local proj = GetCurLevel():FindChildEntity( "proj" )
 local l = CreateLighningEft( proj:GetProjSrc(), dst )
 proj:SetTarget( dst )
 
 local tips = { { "ERROR: Unauthorized accessing.", htx_color_1 },
  { "Don't touch it.", htx_color_1 } }
 local item = tips[RandInt( 0, #tips ) + 1]
 HeadText( item[1], item[2] )
 GetCurLevel():Fail( 0 )
end

function Day4_Password( nSig )
 while true do
  local m = GetCurLevel():GetPawnByName( "sofa" )
  if nSig == 0 then
   if CurTime() == 1 and FEVT( "$pwd_tips" ) then
    RunScenario( function()
     Delay( 45 )
     WaitFor( ScenarioDialogue( 1, "/*------------------------------------", dtx_color_5, -1 ) )
     WaitFor( ScenarioDialogue( 1, "This mail is generated automatically. DO NOT reply.", dtx_color_5, -1 ) )
     WaitFor( ScenarioDialogue( 1, "------------------------------------*/", dtx_color_5, -1 ) )
     WaitFor( ScenarioDialogue( 1, "Hello,", dtx_color_5, -1 ) )
     WaitFor( ScenarioDialogue( 1, "We generated this mail because we detected your frequent", dtx_color_5, -1 ) )
     WaitFor( ScenarioDialogue( 1, "abnormal activities recently. In order to make sure that", dtx_color_5, -1 ) )
     WaitFor( ScenarioDialogue( 1, "the duty of admin will be carried out normally, we need to", dtx_color_5, -1 ) )
     WaitFor( ScenarioDialogue( 1, "temporarily freeze your admin permission and check your sanity", dtx_color_5, -1 ) )
     WaitFor( ScenarioDialogue( 1, "per some procedures. Please follow our instruction to help us", dtx_color_5, -1 ) )
     WaitFor( ScenarioDialogue( 1, "complete our job.", dtx_color_5, -1 ) )
     WaitFor( ScenarioDialogue( 1, "/*-----------------IMPORTANT------------------*/", dtx_color_5, -1 ) )
     WaitFor( ScenarioDialogue( 1, "To fetch back your admin permission, you need to sequentially", dtx_color_5, -1 ) )
     WaitFor( ScenarioDialogue( 1, "input 3 verification code with our hints. All verification codes,", dtx_color_5, -1 ) )
     WaitFor( ScenarioDialogue( 1, "along with their corresponding hints, were generated based on", dtx_color_5, -1 ) )
     WaitFor( ScenarioDialogue( 1, "your personal data to ensure that you can easily complete them", dtx_color_5, -1 ) )
     WaitFor( ScenarioDialogue( 1, "if you are without any mental disorder. Each code consists of", dtx_color_5, -1 ) )
     WaitFor( ScenarioDialogue( 1, "and only of 4 capital letters. Please input them via the keys", dtx_color_5, -1 ) )
     WaitFor( ScenarioDialogue( 1, "on the controller.", dtx_color_5, -1 ) )
     WaitFor( ScenarioDialogue( 1, "/*-----------------------------------*/", dtx_color_5, -1 ) )
     WaitFor( ScenarioDialogue( 1, ">>>CODE 1: Please input your first name.", dtx_color_5, -1 ) )
     WaitFor( ScenarioDialogue( 0, "..................." , dtx_color_1, 60, 6 ) )
     WaitFor( ScenarioDialogue( 0, ".........Are you motherfuckers regarding me as a fool?" , dtx_color_1, -1 ) )
     WaitFor( ScenarioDialogue( 1, ">>>Please complete this code before lunchtime ends.", dtx_color_5, -1 ) )

     local player = GetPlayer()
     WaitFor( ScenarioDialogue( 0, "..................." , dtx_color_1, 60, 6 ) )
     Delay( 45 )
     WaitFor( ScenarioDialogue( 0, "......No no motherfucker..." , dtx_color_1, 60, 6 ) )
     Delay( 45 )
     local myName = EvaluateKeyString( "%myname_str" )
     ScenarioDialogue( 0, myName .. "......Where is the key *" .. myName:sub( 1, 3 ) .. "*?" , dtx_color_1, 1 )
     local pwdUI = GetMasterLevel():ShowInteractionUI( m, "data/interaction/password.pf" )
     pwdUI:SetPassword( "" )
     while GetMasterLevel():GetInteractionUI() do
      coroutine.yield()
     end
     WaitFor( ScenarioDialogue( 0, "........................", dtx_color_1, -1, 3 ) )
     GetMasterLevel():BlackOut( 10, 0 )
     WaitFor( ScenarioDialogue( 0, "........................", dtx_color_1, 60, 3 ) )
     GetMasterLevel():BlackOut( 10, 0 )
     player:PlayState( "move_x" )
     WaitFor( ScenarioDialogue( 0, ".................", dtx_color_1, 30 ) )
     GetMasterLevel():BlackOut( 10, 0 )
     WaitFor( ScenarioDialogue( 0, "...............", dtx_color_1, 10 ) )
     GetMasterLevel():BlackOut( 10, 0 )
     WaitFor( ScenarioDialogue( 0, "......", dtx_color_1, 10 ) )
     GetMasterLevel():BlackOut( 10, 0 )
     player:PlayStateTurnBack( "move_x" )
     for i = 1, 3, 1 do
      Delay( 20 )
      GetMasterLevel():BlackOut( 10, 0 )
     end
     WaitFor( ScenarioDialogue( 0, "I'm mad.", dtx_color_1, 10 ) )
     player:PlayStateTurnBack( "move_up" )
     GetMasterLevel():BlackOut( 10, 0 )
     for i = 1, 5, 1 do
      Delay( 20 )
     GetMasterLevel():BlackOut( 10, 0 )
     end
     WaitFor( ScenarioDialogue( 0, "I'm fucking mad.", dtx_color_1, 5 ) )
     GetMasterLevel():BlackOut( 10, 0 )
     player:PlayStateTurnBack( "move_down" )
     for i = 1, 7, 1 do
      Delay( 12 )
      GetMasterLevel():BlackOut( 10, 2 )
     end
     player:PlayStateTurnBack( "move_down" )
     for i = 1, 10, 1 do
      Delay( 7 )
      GetMasterLevel():BlackOut( 5, 2 )
     end
     WaitFor( ScenarioDialogue( 0, "FUCKING MAD.", dtx_color_1, 5 ) )
     GetMasterLevel():BlackOut( 5, 0 )
     for i0 = 1, 5, 1 do
      player:PlayStateTurnBack( "move_x" )
      for i = 1, 4, 1 do
       Delay( 8 )
       GetMasterLevel():BlackOut( 5, 3 )
      end
     end

     for i0 = 1, 3, 1 do
      player:PlayStateTurnBack( "break" )
      for i = 1, 10, 1 do
       Delay( 10 )
       GetMasterLevel():BlackOut( 5, 5 )
      end
     end
     ScenarioDialogue( 0, "FUUUUUCKIIIIIIINGGGGGG MADDDDDDDDDDDD.", dtx_color_1, 3 )
     GetMasterLevel():BlackOut( 3, 0 )
     for i0 = 1, 15, 1 do
      player:PlayStateTurnBack( "break" )
      for i = 1, 7, 1 do
       Delay( 10 )
       GetMasterLevel():BlackOut( 3, 7 )
      end
     end
     Delay( 5 )
     GetMasterLevel():BlackOut( 180, 0 )
	 
     Delay( 180 )
     WaitFor( ScenarioDialogue( 0, "......NO......", dtx_color_1, -1 ) )
     Delay( 30 )
     GetMasterLevel():BlackOut( 10, 0 )
     Delay( 30 )
     GetMasterLevel():BlackOut( 10, 0 )
     Delay( 30 )
     WaitFor( ScenarioDialogue( 0, "......No, I'm not......", dtx_color_1, -1 ) )
     Delay( 45 )
     GetMasterLevel():BlackOut( 10, 0 )
     Delay( 45 )
     WaitFor( ScenarioDialogue( 0, "......I remembered something......", dtx_color_1, -1 ) )
     Delay( 60 )
     GetMasterLevel():BlackOut( 10, 0 )
     Delay( 60 )
     WaitFor( ScenarioDialogue( 0, "......I think I remembered it......", dtx_color_1, -1 ) )
     Delay( 120 )
     SetKeyInt( "$passed", 1 )
     GetMasterLevel():CheckPoint()
     GetMasterLevel():BlackOut( 120, 60 )
    end )
   else
    local pwdUI = GetMasterLevel():ShowInteractionUI( m, "data/interaction/password.pf" )
    pwdUI:SetPassword( EvaluateKeyString( "day4_pwd" ) )
   end

  elseif nSig == 1 then
   if CurTime() == 2 then
    if not FEVT( "$cheatproof" ) then
	 HeadText( "You fucking cheater.", htx_color_h )
	 GetCurLevel():Fail( 1 )
	 return
	end
    SetKeyString( "day4_pwd", "RIOT" )
    RunScenario( function()
     Delay( 30 )
     WaitFor( ScenarioDialogue( 1, "Verifying......", dtx_color_5, 60, 6  ) )
     WaitFor( ScenarioDialogue( 0, ".................", dtx_color_1, 10, 6 ) )
	 Delay( 60 )
     GetMasterLevel():BlackOut( 20, 0 )
	 Delay( 50 )
     WaitFor( ScenarioDialogue( 0, ".................", dtx_color_1, 10, 6 ) )
	 Delay( 40 )
     GetMasterLevel():BlackOut( 10, 0 )
	 Delay( 40 )
     GetMasterLevel():BlackOut( 10, 0 )
	 Delay( 40 )
     WaitFor( ScenarioDialogue( 1, "......OK.", dtx_color_5, 60 ) )
     WaitFor( ScenarioDialogue( 0, "......Fuck.......", dtx_color_1, 10 ) )
	 Delay( 25 )
     GetMasterLevel():BlackOut( 8, 0 )
	 Delay( 25 )
     GetMasterLevel():BlackOut( 8, 0 )
	 Delay( 25 )
     GetMasterLevel():BlackOut( 8, 0 )
	 Delay( 60 )
     WaitFor( ScenarioDialogue( 0, ".................", dtx_color_1, 10 ) )
	 Delay( 30 )
     GetMasterLevel():BlackOut( 10, 0 )
	 Delay( 30 )
     GetMasterLevel():BlackOut( 10, 0 )
     WaitFor( ScenarioDialogue( 1, ">>>CODE 2: The crime you were convicted of.", dtx_color_5, -1 ) )
     WaitFor( ScenarioDialogue( 0, "...You motherfuckers...", dtx_color_1, 60 ) )
     WaitFor( ScenarioDialogue( 0, "...Want my fucking repentance?", dtx_color_1, 60 ) )
     WaitFor( ScenarioDialogue( 0, "...Fuck off.", dtx_color_1, 60 ) )
	 
     ScenarioDialogue( 0, "Steal..Theft..No, 4 letters......", dtx_color_1, 1 )
     local pwdUI = GetMasterLevel():ShowInteractionUI( m, "data/interaction/password.pf" )
     pwdUI:SetPassword( EvaluateKeyString( "day4_pwd" ) )
     while GetMasterLevel():GetInteractionUI() do
      coroutine.yield()
     end
	 
     WaitFor( ScenarioDialogue( 0, "...Must be some fucking lawyer's term.", dtx_color_1, 60, 3 ) )
     WaitFor( ScenarioDialogue( 0, "..........Fuck it.", dtx_color_1, 60, 3 ) )
	 
     SetKeyInt( "$passed", 1 )
     GetMasterLevel():CheckPoint()
     GetMasterLevel():BlackOut( 30, 0 )
	end )

   elseif CurTime() == 3 then
    if not FEVT( "$cheatproof" ) then
	 HeadText( "You fucking cheater.", htx_color_h )
	 GetCurLevel():Fail( 1 )
	 return
	end
    --SetKeyString( "day4_pwd", "RIOT" )
    RunScenario( function()
     Delay( 60 )
     WaitFor( ScenarioDialogue( 1, "........OK.", dtx_color_5, 60 ) )
     ScenarioDialogue( 1, ">>>CODE 3: Please input the lunch today.", dtx_color_5, -1 )
     local pwdUI = GetMasterLevel():ShowInteractionUI( m, "data/interaction/password.pf" )
     pwdUI:SetPassword( EvaluateKeyString( "day4_pwd" ) )
     while GetMasterLevel():GetInteractionUI() do
      coroutine.yield()
     end
	 
     WaitFor( ScenarioDialogue( 0, ".........", dtx_color_1, 60, 6 ) )
     WaitFor( ScenarioDialogue( 0, "...MASTER.", dtx_color_1, 60, 3 ) )
     WaitFor( ScenarioDialogue( 0, "...MASTER. Are you there?", dtx_color_1, 60, 3 ) )
     WaitFor( ScenarioDialogue( 0, ".........", dtx_color_1, -1, 6 ) )
	 Delay( 60 )
	 
     SetKeyInt( "$passed", 1 )
     GetMasterLevel():CheckPoint()
     GetMasterLevel():BlackOut( 30, 0 )
	end )

   elseif CurTime() == 4 then
   else
    GetPlayer():PlayState( "leave" )
   end
  elseif nSig == 2 then
  elseif nSig == 3 then
   GetPlayer():PlayState( "leave" )
  end
  nSig = coroutine.yield( 1 )
 end
end

function Scenario_Houndman()
 Delay( 15 )
 SetKeyInt( "door_4f_center_c4_open", 1 )
 Delay( 15 )
 GetMasterLevel():BlackOut( 30, 0 )
 local pawn = GetCurLevel():SpawnPawn( 1, 2, 2, 0 )
 GetCurLevel():BeginTracer1( 0, 110 )
 Delay( 40 )
 WaitFor( ScenarioDialogue( 0, "..........", dtx_color_1, -1, 6 ) )
 Delay( 40 )
 for i = 1, 3, 1 do
  GetMasterLevel():BlackOut( 5, 0 )
  Delay( 55 )
 end
 
 WaitFor( ScenarioDialogue( 1, "..........", dtx_color_6, 60, 6 ) )
 GetCurLevel():RemovePawn( pawn )
 pawn = GetCurLevel():SpawnPawn( 0, 2, 2, 0 )
 GetMasterLevel():BlackOut( 30, 0 )
 Delay( 30 )
 WaitFor( ScenarioDialogue( 1, "[Furious growl]", dtx_color_6, -1 ) )
 Delay( 60 )

 local tbl = { { 1, 8, 40 }, { 2, 6, 24 }, { 4, 4, 20 }, { 6, 3, 12 } }
 for i = 1, #tbl, 1 do
  for i1 = 1, tbl[i][1], 1 do
   GetMasterLevel():BlackOut( tbl[i][2], 0 )
   pawn:PlayState( "stand_special" )
   Delay( tbl[i][3] )
   GetMasterLevel():BlackOut( tbl[i][2], 0 )
   pawn:PlayState( "stand" )
   Delay( tbl[i][3] )
  end
 end
 pawn:PlayState( "stand_special" )
 Delay( 60 )
 pawn:PlayState( "stand" )
 SetKeyInt( "__houndman", 1 )
 HeadText( "Shit shit shit..", htx_color_0, 120 )
end

function Scenario_Houndman_End()
 GetCurLevel():EndTracer()
 local pawn = GetCurLevel():GetPawnByName( "__TRACER" )
 if pawn then
  pawn:PlayState( "stand_special" )
  Delay( 60 )
  GetMasterLevel():BlackOut( 30, 0 )
  Delay( 60 )
  pawn:PlayStateSetDir( "stand_special", 1 )
  GetMasterLevel():BlackOut( 5, 0 )
  Delay( 30 )
  pawn:PlayStateSetDir( "move_x_special", 1 )
  GetMasterLevel():BlackOut( 3, 3 )
  Delay( 18 )
  GetMasterLevel():BlackOut( 60, 60 )
  Delay( 80 )
 else
  Delay( 60 )
 end
 WaitFor( ScenarioDialogue( 0, "..........", dtx_color_1, 60, 6 ) )
 WaitFor( ScenarioDialogue( 0, "It went back...", dtx_color_1, -1 ) )
end

function Scenario_Bot_1()
 local b1 = GetCurLevel():GetPawnByName( "b1" )
 Delay( 60 )
 b1:PlayStateSetDir( "stand_ready", 0 )
 WaitFor( ScenarioDialogue( 1, "Disinfectant 5%. Continue executing...", dtx_color_2, 30 ), 30 )
 b1:PlayState( "move_up_ready" )
 Delay( 60 )
 WaitFor( PlayStateAndWait( b1, "move_up" ) )
 Delay( 30 )
 WaitFor( PlayStateAndWait( b1, "disinfect_1" ) )
 b1:PlayState( "disinfect_loop" )
 WaitFor( ScenarioDialogue( 1, "Start disinfecting...", dtx_color_2, 30 ), 90 )
 b1:PlayState( "disinfect_pause" )
 Delay( 30 )
 WaitFor( ScenarioDialogue( 1, "ERROR: Not enough disinfectant.", dtx_color_3, 30 ) )
 WaitFor( ScenarioDialogue( 1, "The process has been pended.", dtx_color_3, 30 ), 90 )
 WaitFor( ScenarioDialogue( 0, "This is not enough...", dtx_color_1, -1 ) )
 Delay( 60 )
end

function Scenario_Bot_2()
 local player = GetPlayer()
 local b2 = GetCurLevel():GetPawnByName( "b2" )
 Delay( 60 )
 WaitFor( ScenarioDialogue( 1, "Disinfectant 100%. Continue executing...", dtx_color_2, 30 ), 30 )
 b2:PlayState( "disinfect_loop" )
 WaitFor( ScenarioDialogue( 1, "..........", dtx_color_2, 30 ), 120 )
 if player:GetPosX() < b2:GetPosX() then
  player:PlayStateSetDir( "move_x", 0 )
 end
 WaitFor( ScenarioDialogue( 1, "disinfecting complete.", dtx_color_2, 30 ), 30 )
 local pickup = GetCurLevel():SpawnPawn1( "data/misc/items/note.pf", b2:GetPosX(), b2:GetPosY(), 0 )
 local pickup1 = GetCurLevel():SpawnPawn1( "data/pickups/stealth.pf", b2:GetPosX(), b2:GetPosY(), 0 )

 WaitFor( PlayStateAndWait( b2, "disinfect_2" ) )
 b2:PlayStateSetDir( "move_down_ready", 1 )
 Delay( 60 )
 WaitFor( PlayStateAndWait( b2, "move_down" ) )
 Delay( 30 )
 WaitFor( PlayStateAndWait( b2, "transform" ) )
 Delay( 90 )

 WaitFor( ScenarioDialogue( 0, ".........", dtx_color_1, -1 ), 30 )
 local tempAI = player:ChangeAI( "data/misc/ai/player_helper_ai_attack.pf" )
 tempAI:SetTarget( pickup:GetPosX(), pickup:GetPosY(), 1 )
 WaitFor( function() return tempAI:IsFinished() end )
 player:ChangeAI( "" )
 Delay( 50 )
 UnlockDoc( "_LAWYER_NOTE" )
end

function Scenario_Day4_2_1()
 local p1 = GetCurLevel():GetPawnByName( "1" )
 local p2 = GetCurLevel():GetPawnByName( "2" )

 WaitFor( ScenarioDialogue( 1, "......Enough of those bullshit.", dtx_color_6a, 60 ) )
 WaitFor( ScenarioDialogue( 1, "You better explain why you are fucking having it.", dtx_color_6a, 90 ) )
 WaitFor( ScenarioDialogue( 1, "Fuck you. You gave me that shit.", dtx_color_6b, 90 ) )
 WaitFor( ScenarioDialogue( 1, "I never give anything to such an asshole like you.", dtx_color_6a, 60 ) )
 WaitFor( ScenarioDialogue( 1, "Where the fuck did you steal it?", dtx_color_6a, 90 ) )
 WaitFor( ScenarioDialogue( 1, "I stole that piece of shit?", dtx_color_6b, 60 ) )
 WaitFor( ScenarioDialogue( 1, "You are fucking calling me the thief, you crook?", dtx_color_6b, 60 ) )
 WaitFor( ScenarioDialogue( 1, "Who the fuck will give a shit to that crap?", dtx_color_6b, 60 ) )
 WaitFor( ScenarioDialogue( 1, "That shit for my disinfectant? and then call me the thief?", dtx_color_6b, 120 ) )
 WaitFor( ScenarioDialogue( 1, "I've never fucking GIVEN it. It is MINE.", dtx_color_6a, 60 ) )
 WaitFor( ScenarioDialogue( 1, "Hand it over. Now. You fucking thief.", dtx_color_6a, 90 ) )
 WaitFor( ScenarioDialogue( 1, "Fuck off. Give me back my disinfectant first.", dtx_color_6b, 90 ) )
 WaitFor( ScenarioDialogue( 1, "I have NO fucking disinfectant.", dtx_color_6b, 90 ) )
 WaitFor( ScenarioDialogue( 1, "I never NEED that thing. Hand it fucking over.", dtx_color_6a, 120 ) )
 WaitFor( ScenarioDialogue( 1, "Let me guess. You are just having trouble taking it out?", dtx_color_6b, 90 ) )
 WaitFor( ScenarioDialogue( 1, "Maybe you should insert it less deeply?", dtx_color_6b, 90 ) )
 WaitFor( ScenarioDialogue( 1, ".........", dtx_color_6a, 60, 6 ) )

 WaitFor( PlayStateAndWait( p1, "attack_up" ) )
 WaitFor( ScenarioDialogue( 1, "I poured it onto your fucking cock.", dtx_color_6a, 30 ), 60 )
 p1:PlayState( "attack_up" )
 Delay( 40 )
 WaitFor( ScenarioDialogue( 1, "And I rolled it like a joystick.", dtx_color_6a, 30 ), 120 )
 p1:PlayState( "attack_up" )
 Delay( 20 )
 ScenarioDialogue( 1, "Then I suddenly pulled it out. Like a frozen drumstick.", dtx_color_6a, 30 )
 Delay( 50 )
 WaitFor( PlayStateAndWait( p2, "attack_down" ) )
 WaitFor( ScenarioDialogue( 1, "Fuck your dick you motherfucker.", dtx_color_6b, 30 ), 60 )
 p2:PlayState( "attack_down" )
 Delay( 40 )
 WaitFor( ScenarioDialogue( 1, "Go fucking yourself if you feel fucking bored.", dtx_color_6b, 30 ), 120 )
 p2:PlayState( "attack_down" )
 Delay( 60 )
 ScenarioDialogue( 1, "Want me to show you how to fuck yourself?", dtx_color_6b, 30 )
 Delay( 30 )
 p1:PlayState( "attack_up" )
 Delay( 40 )
 ScenarioDialogue( 1, "I fucking know it. Just like you did to your mother.", dtx_color_6a, 20 )
 Delay( 20 )
 p2:PlayState( "attack_down" )
 Delay( 40 )
 ScenarioDialogue( 1, "No, I think it was YOUR mother.", dtx_color_6b, 30 )
 Delay( 30 )
 p2:PlayState( "attack_down" )
 Delay( 40 )
 ScenarioDialogue( 1, "Maybe you forgot it because my stick hit your head too hard.", dtx_color_6b, 60 )
 Delay( 60 )
 p1:PlayState( "attack_up" )
 Delay( 40 )
 ScenarioDialogue( 1, "Sorry I forgot you don't like old women.", dtx_color_6a, 30 )
 Delay( 30 )
 p1:PlayState( "attack_up" )
 Delay( 40 )
 WaitFor( ScenarioDialogue( 1, "You love little girls. That's why you're here.", dtx_color_6a, -1 ) )

 Delay( 90 )
 WaitFor( ScenarioDialogue( 1, ".........", dtx_color_6b, 60, 6 ), 60 )
 for i = 1, 3, 1 do
  p2:PlayState( "attack_down" )
  Delay( 65 )
 end
 WaitFor( ScenarioDialogue( 1, "Say that again and I'll crumb your balls.", dtx_color_6b, 60, 6 ), -1 )
 WaitFor( ScenarioDialogue( 1, "You pervert of no balls dare threaten me?", dtx_color_6a, 10 ), 40 )
 for i = 1, 2, 1 do
  p1:PlayState( "attack_up" )
  Delay( 65 )
 end
 WaitFor( ScenarioDialogue( 1, "Stuff all your videos into your asshole, you sex freak.", dtx_color_6a, -1 ) )
 WaitFor( ScenarioDialogue( 1, "Fuck yourself with your hypocrisy.", dtx_color_6b, 60 ), 90 )
 WaitFor( ScenarioDialogue( 1, "Don't accuse me while pretending you don't like it.", dtx_color_6b, 60 ), 90 )
 WaitFor( ScenarioDialogue( 1, "After all we are scums of the same kind. All of us.", dtx_color_6b, 60, 3 ), -1 )
 p1:PlayState( "attack_up" )
 Delay( 30 )
 WaitFor( ScenarioDialogue( 1, "At least It wasn't me who was caught on the spot.", dtx_color_6a, -1 ), 70 )
 WaitFor( ScenarioDialogue( 1, "No I wasn't either. At least that's not my judgement.", dtx_color_6b, 60, 2 ), -1 )
 WaitFor( ScenarioDialogue( 1, "It's the same as yours. You know it.", dtx_color_6b, 60, 3 ), -1 )
 WaitFor( ScenarioDialogue( 1, "We are all the same. All fucking same.", dtx_color_6b, 60, 6 ), -1 )
 p1:PlayState( "attack_up" )
 Delay( 20 )
 ScenarioDialogue( 1, "Shut up.", dtx_color_6a, 80 )
 Delay( 80 )
 p1:PlayState( "attack_up" )
 Delay( 20 )
 ScenarioDialogue( 1, "SHUUUUUT UP.", dtx_color_6a, 80 )
 Delay( 80 )
 p1:PlayState( "attack_up" )
 Delay( 20 )
 ScenarioDialogue( 1, "SHUT THE FUCK UP.", dtx_color_6a, 80 )
 p1:GetAI():SetSpecialBehaviorEnabled( true )
 Delay( 120 )
end

function Day4_2_1_AISpecialBehavior( pawn, nCurDir )
 local player = GetPlayer()
 if not player:IsHidden() or pawn:GetAI():IsSeePlayer() then
  return -1
 end
 return 8, nCurDir
end

function Day4_2_1_AISpecialFunc( ai, pawn )
 local context = { bFin = false }
 local coCheckStateTransits1 = coroutine.create( function( nCurState, nCurDir, bFinished )
  -------------------  nCurState, nCurDir, bFinished = Delay( 20 )
  Delay( 20 )
  HeadText( "Fuck, fuck...", htx_color_6b )
  Delay( 180, 1, 1 )
  PlaySoundEffect( "door_hit" )
  Delay( 60 )
  HeadText( "Stop it idiot.", htx_color_6b )
  Delay( 120, 0, 0 )
  PlaySoundEffect( "door_hit" )
  Delay( 60 )
  HeadText( "Useless.", htx_color_6b )
  Delay( 120 )
  Delay( 60, 2, 1 )
  PlaySoundEffect( "door_hit" )
  Delay( 60 )
  HeadText( "...Fuck it went wrong again.", htx_color_6b )
  Delay( 60 )
  PlaySoundEffect( "door_hit" )
  Delay( 60 )
  HeadText( "Reset it mecha jackass. Don't you have eyes?", htx_color_6b )
  for i = 1, 4, 1 do
   Delay( 20, 0, ( i - 1 ) % 2 )
  end
  Delay( 90 )
  HeadText( "Did your creator plug into the wrong hole?", htx_color_6b )
  Delay( 150 )
  HeadText( "..........", htx_color_6b )
  Delay( 90 )
  Delay( 60, 2, 1 )
  HeadText( "Hi Kucha. Please reset.", htx_color_6b )
  local function ResetSecurity()
   Delay( 90 )
   HeadText( "Security system reseting...", htx_color_sys, 120 )
   Delay( 90 )
   Signal( GetCurLevel():FindChildEntity( "tutorial_follow" ), 0 )
   Signal( GetCurLevel():FindChildEntity( "tutorial_follow" ), 2 )
  end
  LevelRegisterUpdate1( ResetSecurity )
  Delay( 300 )

  local bKnock = true
  LevelRegisterUpdate1( function()
   while bKnock do
    PlaySoundEffect( "door_hit" )
    Delay( 240 )
   end
  end )

  Delay( 60 )
  HeadText( "Fuck you. Are't you going to rest for a fucking while?", htx_color_6b )
  Delay( 120, 0, 0 )
  HeadText( "...What the fuck did you say?", htx_color_6b )
  Delay( 120, 3, 0 )
  HeadText( "...Stop fucking me. Go fuck Lawyer " .. NAME_LAWYER .. ".", htx_color_6b )
  Delay( 120, 3, 0 )
  HeadText( "He's the one who fucked us all.", htx_color_6b )
  Delay( 100 )
  HeadText( "He told us to do as he said.", htx_color_6b )
  Delay( 90 )
  HeadText( "He made our sentences.", htx_color_6b )
  Delay( 80 )
  HeadText( "He packed us together to be sent here.", htx_color_6b )
  Delay( 80 )
  HeadText( "Why don't you go fuck his guts out of his asshole for us?", htx_color_6b )
  Delay( 150 )
  HeadText( "......What?", htx_color_6b )
  bKnock = false
  Delay( 150 )
  HeadText( "......Say it again?", htx_color_6b )
  SetKeyInt( "4f_center_room_2a_d1", 0 )
  Delay( 120, 1, 0 )
  for i = 1, 2, 1 do
   LevelRegisterUpdate1( function() GetCurLevel():SpawnPawn1( "data/enemies/enemy_attack_0.pf", pawn:GetToX(), pawn:GetToY(), 1 ) end )
   Delay( 60, 4, 0 )
  end
  HeadText( "...Oh fuck...", htx_color_6b )
  Delay( 120 )
  for i = 1, 4, 1 do
   LevelRegisterUpdate1( function() GetCurLevel():SpawnPawn1( "data/enemies/enemy_attack_0.pf", pawn:GetToX(), pawn:GetToY(), 1 ) end )
   Delay( 50, 4, 0 )
  end
  Delay( 40, 1, 1 )
  HeadText( "...Damn it...", htx_color_6b )
  Delay( 60 )

  if GetCurLevel():FindChildEntity( "tutorial_follow" ):IsAnythingAbnormal() then
   HeadText( "...Screw this...What's fucking wrong with it today?", htx_color_6b )
   Delay( 120 )
   HeadText( "...Kucha, reset.", htx_color_6b )
   LevelRegisterUpdate1( ResetSecurity )
   Delay( 240 )
  else
   Delay( 40 )
   HeadText( "...This machine better not get me in trouble.", htx_color_6b )
   Delay( 120 )
  end

  Delay( 40, 0, 0 )
  HeadText( "...Come in, motherfucker. Get inside.", htx_color_6b )
  Delay( 100 )
  HeadText( "Don't just stand there. Get in and enjoy yourself.", htx_color_6b )
  Delay( 100 )
  HeadText( "We can watch some cute videos together.", htx_color_6b )
  Delay( 100 )
  HeadText( "Or what about your favorite colletion?", htx_color_6b )
  Delay( 100 )
  HeadText( "..........", htx_color_6b )
  Delay( 100 )
  HeadText( "...Dare you not.", htx_color_6b )
  Delay( 140 )
  HeadText( "...Ok. So where were we? Did you remember?", htx_color_6b )
  Delay( 100 )
  HeadText( "...Yes, that's it. I know it. You are fucking right.", htx_color_6b )
  Delay( 120 )
  HeadText( "...Yes yes yes. Don't remind me. I really fucking know it.", htx_color_6b )
  Delay( 120 )
  HeadText( "...Very good. That's the very fucking point. I loved her.", htx_color_6b )
  Delay( 120 )
  HeadText( "...While her parents didn't. Isn't it fucking fair?", htx_color_6b )
  Delay( 120 )
  HeadText( "...Yes yes. Sick enough to love her.", htx_color_6b )
  Delay( 120 )
  HeadText( "Sick enough to protect her from being used as a body shield.", htx_color_6b )
  Delay( 120 )
  HeadText( "I'm a fucking pervert. A fucking sex freak. Any questions?", htx_color_6b )
  Delay( 150 )
  HeadText( "...Then fuck off. We have nothing to say anymore.", htx_color_6b )
  Delay( 100 )
  SetKeyInt( "4f_center_room_2a_d1", 1 )
  HeadText( "...Fuck you. I'm not listening. Question time is over.", htx_color_6b )
  Delay( 60 )
  PlaySoundEffect( "door_hit" )
  Delay( 30 )
  Delay( 60, 2, 1 )
  HeadText( "...I'm watching TV. I'm not listening to your bullshit.", htx_color_6b )
  Delay( 60 )
  GetCurLevel():GetPawnByName( "tv" ):FindChildEntity( "screen" ):SetVisible( true )
  GetCurLevel():BeginNoise( "noise_tv" )
  Delay( 30 )
  PlaySoundEffect( "door_hit" )

  Delay( 60 )
  HeadText( "...I'm NOT LISTENING. Whatever you say.", htx_color_6b )
  Delay( 90 )
  HeadText( '"UPCOMING: Mystery of the origin of Yellow August"', htx_color_5 )
  Delay( 110 )
  HeadText( '"FROM -- AUTO RECOMMENDATION"', htx_color_5 )
  Delay( 80 )
  PlaySoundEffect( "door_hit" )
  Delay( 30 )
  HeadText( "...I'm NOT FUCKING LISTENING.", htx_color_6b )
  Delay( 60 )
  HeadText( '"...As the most decisive event in the second half of the last century..."', htx_color_5 )
  Delay( 200 )
  HeadText( '"...The whole country is burning...Prisons are overflowing..."', htx_color_5 )
  Delay( 200 )
  HeadText( "[Lots of noise and glitch]", htx_color_5 )
  Delay( 90 )
  HeadText( "..........", htx_color_6b )
  Delay( 60 )
  HeadText( '"...About the trigger for the crisis there are many controversies..."', htx_color_5 )
  Delay( 200 )
  HeadText( "[Glitch again]", htx_color_5 )
  Delay( 90 )
  HeadText( '"...He said he witnessed..."', htx_color_5 )
  Delay( 100 )
  HeadText( "[More and more glitch]", htx_color_5 )
  Delay( 60 )
  HeadText( '"...I was among the protest and was surrounded by people..."', htx_color_5 )
  Delay( 150 )
  HeadText( '"...the parade stopped. I saw they formed two walls..."', htx_color_5 )
  Delay( 150 )
  HeadText( "...NOT FUCKING LISTENING.", htx_color_6b )
  Delay( 60 )
  HeadText( '"...Someone was arrested. The girl was sitting there crying..."', htx_color_5 )
  Delay( 150 )
  HeadText( '"...I walked away quickly. Soon I heard screaming..."', htx_color_5 )
  Delay( 150 )
  HeadText( "[The rest became totally illegible]", htx_color_5 )
  Delay( 90 )
  
  if GetCurLevel():FindChildEntity( "tutorial_follow" ):IsAnythingAbnormal() then
   HeadText( "NOT......What the hell is this piece of scrap doing?", htx_color_6b )
   Delay( 100 )
   HeadText( "...Kucha, SHUTDOWN.", htx_color_6b )
   Delay( 100 )
   HeadText( "ERROR: inadequate permissions.", htx_color_1 )
   Delay( 100 )
   HeadText( "THEN FUCKING RESET.", htx_color_6b )
   Delay( 100 )
   HeadText( "...Reset, I mean.", htx_color_6b )
   LevelRegisterUpdate1( ResetSecurity )
   Delay( 240 )
   HeadText( "...Enough. I've had fucking enough.", htx_color_6b )
  else
   HeadText( "NOT...Enough.", htx_color_6b )
   Delay( 120 )
   HeadText( "I've had enough.", htx_color_6b )
  end
  Delay( 45 )
  GetCurLevel():GetPawnByName( "tv" ):FindChildEntity( "screen" ):SetVisible( false )
  GetCurLevel():EndNoise( "noise_tv" )
  Delay( 45 )
  Delay( 120, 3, 0 )
  HeadText( "...Fucking gone finally.", htx_color_6b )

  context.bFin = true
 end )

 local FuncLoop = function( name, ... )
  if not GetPlayer():IsHidden() or ai:IsSeePlayer() then
   HeadText( "...What's that?", dtx_color_6b, 240 )
   SetKeyInt( "4f_center_room_2a_d1", 0 )
   context.bFin = true
   return false
  end
  if name == "CheckStateTransits1" then
   local b, nCurState, nCurDir = coroutine.resume( coCheckStateTransits1, ... )
   if not nCurState then
    return true, -1
   end
   return true, nCurState, nCurDir
  end
 end

 local params = { coroutine.yield() }
 while true do
  local ret = { FuncLoop( table.unpack( params ) ) }
  if context.bFin then
   return table.unpack( ret )
  end
  params = { coroutine.yield( table.unpack( ret ) ) }
 end
end

function Day5_Get_Neural_Pulse()
 if CurDay() ~= 5 then return end
 RunScenario( function()
  local p = GetPlayer()
  local pos0 = { ( p:GetPosX() + 1 ) * LEVEL_GRID_SIZE_X, p:GetPosY() * LEVEL_GRID_SIZE_Y + 48 }
  local dst = { { pos0[1] - 256, pos0[2] }, { pos0[1] + 256, pos0[2] }, { pos0[1] - 128, pos0[2] + 224 },
  { pos0[1] + 128, pos0[2] + 224 }, { pos0[1] + 128, pos0[2] - 224 }, { pos0[1] - 128, pos0[2] - 224 } }
  local l = {}
  for i = 1, 6, 1 do
   l[i] = CreateLighningEft( pos0, dst[i] )
  end
  GetMasterLevel():InterferenceStripEffect( 1, 2 )
  Delay( 80 )
  for i = 1, 6, 1 do
   l[i]:SetParentEntity( nil )
  end
  GetMasterLevel():BlackOut( 40, 40 )
  GetMasterLevel():InterferenceStripEffect( 0, 0 )
  PlayerPickUp( "data/pickups/neural_pulse.pf" )
  GetPlayer():PlayState( "leave" )
 end )
end