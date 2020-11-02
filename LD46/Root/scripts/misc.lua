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
 GetCurLevel():ScriptForEachEnemy( function( enemy )
  local posX1 = enemy:GetPosX()
  local posY1 = enemy:GetPosY()
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

  posX1 = enemy:GetToX()
  posY1 = enemy:GetToY()
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
     WaitFor( ScenarioDialogue( 1, ">>>Please complete this code to advance.", dtx_color_5, -1 ) )

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
 b1:PlayState( "disinfect_1" )
 WaitFor( ScenarioDialogue( 1, "Start disinfecting...", dtx_color_2, 30 ), 90 )
 b1:PlayState( "disinfect_pause" )
 Delay( 30 )
 WaitFor( ScenarioDialogue( 1, "ERROR: Not enough disinfectant.", dtx_color_3, 30 ) )
 WaitFor( ScenarioDialogue( 1, "The process has been pended.", dtx_color_3, 30 ), 90 )
 WaitFor( ScenarioDialogue( 0, "This is not enough...", dtx_color_1, -1 ) )
 Delay( 60 )
end

function Scenario_Bot_2()
 local b2 = GetCurLevel():GetPawnByName( "b2" )
end