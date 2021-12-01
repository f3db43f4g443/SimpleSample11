function Day1_Following_Bug()
 if CurDay() > 1 or GetLabelKey( "_BUG_3" ) > 0 then return false end
 local player = GetPlayer()
 if GetCurLevel():GetGridExit( player:GetCurStateDestX(), player:GetCurStateDestY() ) <= 0 then return false end
 
 local a1 = GetLabelKey( "_BUG_1" )
 local a2 = GetLabelKey( "_BUG_2" )
 local a3 = 1
 TransferTo( "", 0, 0, 0, -1, 1 )
 TransferOpr( function()
  FEVT( "$sc1" )
  LevelRegisterUpdate( function()
   for i = 12, 0, -1 do
    GetMasterLevel():GetMainUI():ShowFreezeEft( i )
    Delay( 3 )
   end
  end )
  HeadText( "It will be fixed. For now please just follow my instruction." )
  SetLabelKey( "_BUG_1", a1 )
  SetLabelKey( "_BUG_2", a2 )
  SetLabelKey( "_BUG_3", a3 )
  if a1 > 0 and a2 > 0 and a3 > 0 then
   SecretFound()
  end
 end )
 return true
end

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
 WaitFor( ScenarioDialogue( 1, "...An event that requires some procedures to handle.", dtx_color_h, -1 ) )
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

function Scenario_SelfRoom_4_5()
 local proj = GetCurLevel():FindChildEntity("proj")
 local player = GetPlayer()
 local src = proj:GetProjSrc()
 local dst = { ( player:GetPosX() + 1 ) * LEVEL_GRID_SIZE_X, ( player:GetPosY() + 1 ) * LEVEL_GRID_SIZE_Y }
 local l = CreateLighningEft( src, dst )
 player:PlayState( "break" )
 GetMasterLevel():InterferenceStripEffect( 1, 1 )
 Delay( 90 )
 GetMasterLevel():InterferenceStripEffect( 0, 0 )
 l:SetParentEntity( nil )

 WaitFor( ScenarioDialogue( 1, "Finish it. Now.", dtx_color_h, -1 ), 60 )
 WaitFor( ScenarioDialogue( 0, "..........", dtx_color_1, -1, 6 ), 60 )
 WaitFor( ScenarioDialogue( 0, "...Sorry, I can't.", dtx_color_1, -1, 3 ), 60 )
 Delay( 90 )
 WaitFor( ScenarioDialogue( 1, "..........", dtx_color_h, -1, 6 ), 60 )
 WaitFor( ScenarioDialogue( 1, "....What?", dtx_color_h, 60, 2 ) )
 WaitFor( ScenarioDialogue( 1, "Are you kidding me?", dtx_color_h, -1, 2 ), 60 )
 WaitFor( ScenarioDialogue( 0, "...No just one thing I need to know...", dtx_color_1, -1, 2 ), 90 )
 WaitFor( ScenarioDialogue( 0, "...What's my name?", dtx_color_1, -1, 2 ), 90 )

 WaitFor( ScenarioDialogue( 1, "..........", dtx_color_h, -1, 6 ), 60 )
 WaitFor( ScenarioDialogue( 1, "Are you asking me?", dtx_color_h, 60, 2 ) )
 WaitFor( ScenarioDialogue( 1, "Are you fucking mad?", dtx_color_h, -1, 2 ), 90 )
 WaitFor( ScenarioDialogue( 0, "......Kinda, I think.", dtx_color_1, -1, 2 ), 60 )
 Delay( 90 )
 WaitFor( ScenarioDialogue( 1, "..........", dtx_color_h, -1, 6 ), 60 )
 WaitFor( ScenarioDialogue( 1, "I've already told you.", dtx_color_h, 60, 2 ), 60 )
 dst = { ( player:GetPosX() + 1 ) * LEVEL_GRID_SIZE_X, ( player:GetPosY() + 1 ) * LEVEL_GRID_SIZE_Y }
 l = CreateLighningEft( src, dst )
 player:PlayState( "break" )
 GetMasterLevel():InterferenceStripEffect( 1, 1 )
 Delay( 90 )
 GetMasterLevel():InterferenceStripEffect( 0, 0 )
 l:SetParentEntity( nil )
 Delay( 60 )
 WaitFor( ScenarioDialogue( 1, "You must not ask questions.", dtx_color_h, 90 ) )
 WaitFor( ScenarioDialogue( 1, "Don't try to get smart with me.", dtx_color_h, -1 ), 60 )
 Delay( 60 )
 WaitFor( ScenarioDialogue( 0, "..........", dtx_color_1, -1, 6 ), 60 )
 Delay( 60 )
 WaitFor( ScenarioDialogue( 0, "...No it's not a question.", dtx_color_1, -1, 2 ), 60 )
 WaitFor( ScenarioDialogue( 0, "...I just want to know...", dtx_color_1, -1, 2 ), 60 )
 WaitFor( ScenarioDialogue( 0, "Do you know the answer?", dtx_color_1, -1, 2 ), 60 )
 WaitFor( ScenarioDialogue( 1, "So it's a fucking test?", dtx_color_h, 60 ) )
 WaitFor( ScenarioDialogue( 1, "I certainly fucking know.", dtx_color_h, 60 ) )
 WaitFor( ScenarioDialogue( 1, "But why must I...", dtx_color_h, 60 ) )
 WaitFor( ScenarioDialogue( 0, "...Well to be honest,", dtx_color_1, -1, 2 ), 60 )
 WaitFor( ScenarioDialogue( 0, "...I think I'm actually mad.", dtx_color_1, -1, 2 ), 60 )
 WaitFor( ScenarioDialogue( 1, "What shit are you fucking talking about?", dtx_color_h, 60 ) )
 WaitFor( ScenarioDialogue( 1, "That's not funny at all.", dtx_color_h, -1 ) )
 WaitFor( ScenarioDialogue( 0, "...Seems you don't believe me. Do you?", dtx_color_1, -1, 2 ), 90 )
 WaitFor( ScenarioDialogue( 0, "...I'm serious, I mean.", dtx_color_1, -1, 2 ), 90 )
 Delay( 120 )
 dst = { ( player:GetPosX() + 1 ) * LEVEL_GRID_SIZE_X, ( player:GetPosY() + 1 ) * LEVEL_GRID_SIZE_Y }
 l = CreateLighningEft( src, dst )
 player:PlayState( "break" )
 GetMasterLevel():InterferenceStripEffect( 1, 1 )
 Delay( 90 )
 GetMasterLevel():InterferenceStripEffect( 0, 0 )
 l:SetParentEntity( nil )
 WaitFor( ScenarioDialogue( 1, "No you are not.", dtx_color_h, -1 ) )
 WaitFor( ScenarioDialogue( 1, "Just fucking go finish it.", dtx_color_h, -1 ) )
 WaitFor( ScenarioDialogue( 0, "...Yes I am. Really.", dtx_color_1, -1, 2 ), 90 )
 WaitFor( ScenarioDialogue( 0, "...Unless I could have some help.", dtx_color_1, -1, 2 ), 90 )
 WaitFor( ScenarioDialogue( 0, "You don't want it go that way, right?.", dtx_color_1, -1, 2 ), 90 )
 Delay( 120 )
 dst = { ( player:GetPosX() + 1 ) * LEVEL_GRID_SIZE_X, ( player:GetPosY() + 1 ) * LEVEL_GRID_SIZE_Y }
 l = CreateLighningEft( src, dst )
 player:PlayState( "break" )
 GetMasterLevel():InterferenceStripEffect( 1, 1 )
 Delay( 90 )
 GetMasterLevel():InterferenceStripEffect( 0, 0 )
 l:SetParentEntity( nil )
 WaitFor( ScenarioDialogue( 1, "Stop those fucking tricks.", dtx_color_h, 90 ) )
 WaitFor( ScenarioDialogue( 1, "Input the code on your own. No cheat allowed.", dtx_color_h, -1 ) )
 Delay( 120 )
 WaitFor( ScenarioDialogue( 0, "..........", dtx_color_1, -1, 6 ), 60 )
 Delay( 120 )
 WaitFor( ScenarioDialogue( 0, "...No that's no cheat.", dtx_color_1, -1, 2 ), 90 )
 WaitFor( ScenarioDialogue( 1, "That's certainly fucking...", dtx_color_h, 90 ), 90 )
 Delay( 60 )
 WaitFor( ScenarioDialogue( 1, "..........", dtx_color_h, -1, 6 ), 60 )
 WaitFor( ScenarioDialogue( 1, "...What did you say?", dtx_color_h, -1 ), 60 )
 WaitFor( ScenarioDialogue( 0, "...That's nothing to do with the code.", dtx_color_1, -1, 2 ), 90 )
 WaitFor( ScenarioDialogue( 0, "...It's about my fucking CRIME.", dtx_color_1, -1, 6 ), 90 )
 Delay( 120 )
 WaitFor( ScenarioDialogue( 1, "..........", dtx_color_h, -1, 6 ), 60 )
 Delay( 120 )
 WaitFor( ScenarioDialogue( 1, "......................", dtx_color_h, -1, 6 ), 60 )
 Delay( 120 )
 
 dst = { ( player:GetPosX() + 1 ) * LEVEL_GRID_SIZE_X, ( player:GetPosY() + 1 ) * LEVEL_GRID_SIZE_Y }
 l = CreateLighningEft( src, dst )
 player:PlayState( "move_up" )
 GetMasterLevel():InterferenceStripEffect( 1, 1 )
 Delay( 90 )
 GetMasterLevel():InterferenceStripEffect( 0, 0 )
 l:SetParentEntity( nil )
 ScenarioDialogue( 1, "Go. Fucking end this.", dtx_color_h, -1, 2 )
 Delay( 120 )
 dst = { ( player:GetPosX() + 1 ) * LEVEL_GRID_SIZE_X, ( player:GetPosY() + 1 ) * LEVEL_GRID_SIZE_Y }
 l = CreateLighningEft( src, dst )
 player:PlayStateTurnBack( "move_up" )
 GetMasterLevel():InterferenceStripEffect( 1, 1 )
 Delay( 90 )
 GetMasterLevel():InterferenceStripEffect( 0, 0 )
 l:SetParentEntity( nil )
 ScenarioDialogue( 1, "Go.", dtx_color_h, -1, 3 )
 Delay( 120 )
 dst = { ( player:GetPosX() + 1 ) * LEVEL_GRID_SIZE_X, ( player:GetPosY() + 1 ) * LEVEL_GRID_SIZE_Y }
 l = CreateLighningEft( src, dst )
 player:PlayState( "move_up" )
 GetMasterLevel():InterferenceStripEffect( 1, 1 )
 Delay( 90 )
 GetMasterLevel():InterferenceStripEffect( 0, 0 )
 l:SetParentEntity( nil )
 ScenarioDialogue( 1, "Be quick.", dtx_color_h, -1 )
 Delay( 120 )
end

function Day3_Progress( nNode, pawn )
 local nProgress = EvaluateKeyInt( "day3_progress" ) + 1
 SetKeyInt( "day3_progress", nProgress )
 GetMasterLevel():RemoveLevelMark( "mark_" .. tostring( nNode ) )
 local player = GetPlayer()
  
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
 WaitFor( RunScenarioAndWait( function()
  WaitFor( ScenarioDialogue( 1, "Admin logging in..........OK.", dtx_color_2, 60, 6 ) )
  WaitFor( ScenarioDialogue( 1, "Visiting permission node " .. tostring( nNode ) .. "...", dtx_color_2, 60, 6 ) )
  WaitFor( ScenarioDialogue( 1, "Permission granted.", dtx_color_2, 60 ) )
  WaitFor( ScenarioDialogue( 1, "Permission Tokens: " .. tostring( nProgress ) .. ".", dtx_color_2, 60 ) )
  WaitFor( ScenarioDialogue( 1, tostring( 3 - nProgress ) .. " permission tokens left.", dtx_color_2, -1 ) )

  if nProgress == 1 then
   WaitFor( ScenarioDialogue( 0, "I must send a report now...", dtx_color_1, -1 ) )
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
 
 Func( 0, 30 )
 GetMasterLevel():BlackOut( 10, 0 )
 player:SetHp( player:GetMaxHp() )
 GetMasterLevel():Respawn()
 GetMasterLevel():PushPlayerData()
 SetKeyInt( "hub_dest", nNode + 1 )
 local n = tonumber( pawn:GetTag( "target1" ) )
 GetMasterLevel():TransferBy( n, -2 )
end

function Day3_unknown_btn_1()
 local Delay_Down = function( b )
  while EvaluateKeyInt( "$btn" ) == 0 do
   coroutine.yield()
  end
  if not b then SetKeyInt( "$btn", 0 ) end
 end
 local Delay_Up = function()
  while EvaluateKeyInt( "$btn" ) == 1 do
   coroutine.yield()
  end
 end
 local proj = GetCurLevel():FindChildEntity("proj")
 local player = GetPlayer()
 if GetLabelKey( "_COIN_1" ) == 0 then
  Delay_Down() HeadText( "Mom?", htx_color_0, 0, true )
  Delay_Down() HeadText( "You're back too late.", htx_color_h, 0, true )
  Delay_Down() HeadText( "Where did you go?", htx_color_h, 0, true )
  Delay_Down() HeadText( "...Nowhere.", htx_color_0, 0, true )
  Delay_Down() HeadText( "I need some money.", htx_color_0, 0, true )
  Delay_Down() HeadText( "...No.", htx_color_h, 0, true )
  Delay_Down() HeadText( "We have no money left.", htx_color_h, 0, true )
  Delay_Down() HeadText( "Didn't father send money back?", htx_color_0, 0, true )
  Delay_Down() HeadText( "..........", htx_color_h, 0, true )
  Delay_Down() PlaySoundEffect( "sfx_1" ) GetMasterLevel():BlackOut( 20, 0 ) HeadText( "...NO.", htx_color_h, 0, true )
  Delay_Down() HeadText( "Stop talking about him.", htx_color_h, 0, true )
  Delay_Down() HeadText( "..........", htx_color_0, 0, true )
  Delay_Down() HeadText( "Stay home. Don't go out.", htx_color_h, 0, true )
  Delay_Down() HeadText( "It's too late.", htx_color_h, 0, true )
  Delay_Down() Signal( proj, -1 )
  Delay_Down( true ) HeadText( "..........", htx_color_0, 0, true )
  Delay_Up() SetKeyInt( "$d2", 1 )
 else
  Delay_Down() HeadText( "..........", htx_color_h, 0, true )
  Delay_Down() HeadText( "Where did you get it?", htx_color_h, 0, true )
  Delay_Down() HeadText( "You went into fighting again?", htx_color_h, 0, true )
  Delay_Down() HeadText( "...Yes. What's wrong?", htx_color_0, 0, true )
  Delay_Down() HeadText( "...You're ruining yourself...", htx_color_h, 0, true )
  Delay_Down() HeadText( "...You're killing yourself...", htx_color_h, 0, true )
  Delay_Down() HeadText( "...Just like my father?", htx_color_0, 0, true )
  Delay_Down() HeadText( "I told you NOT to mention him.", htx_color_h, 0, true )
  Delay_Down() HeadText( "Yes I will.", htx_color_0, 0, true )
  Delay_Down() HeadText( "He will be proud of me.", htx_color_0, 0, true )
  Delay_Down() HeadText( "When he comes back...", htx_color_0, 0, true )
  Delay_Down() PlaySoundEffect( "sfx_1" ) GetMasterLevel():BlackOut( 20, 0 ) HeadText( "STOP IT.", htx_color_h, 0, true )
  Delay_Down() HeadText( "He......", htx_color_h, 0, true )
  Delay_Down() HeadText( "..........", htx_color_0, 0, true )
  Delay_Down() HeadText( "He's......", htx_color_h, 0, true )
  Delay_Down() HeadText( "HE'LL NEVER BE BACK ANY MORE.", htx_color_h, 0, true )
  Delay_Down() Signal( proj, -1 ) SetKeyInt( "$d1", 1 )
  while player:GetToX() ~= 10 or player:GetToY() ~= 4 do coroutine.yield() end
  HeadText( "" )
  RunScenario( function()
   proj:Follow( player )
   Delay( 80 )
   WaitFor( ScenarioDialogue( 1, "...Buy some flour. No alcohol.", dtx_color_h, -1, 2 ) )
   Delay( 120 )
   WaitFor( ScenarioDialogue( 1, "......Please.", dtx_color_h, -1, 6 ) )
  end )
 end
end

function Day3_unknown_btn_2()
 local Delay_Down = function( b )
  while EvaluateKeyInt( "$btn" ) == 0 do
   coroutine.yield()
  end
  if not b then SetKeyInt( "$btn", 0 ) end
 end
 local Delay_Up = function()
  while EvaluateKeyInt( "$btn" ) == 1 do
   coroutine.yield()
  end
 end
 local proj = GetCurLevel():FindChildEntity("proj")
 local player = GetPlayer()
 if GetLabelKey( "_COIN_1" ) == 0 then
  Delay_Down() HeadText( "Good morning.", htx_color_h, 0, true )
  Delay_Down() HeadText( "You are awake.", htx_color_h, 0, true )
  Delay_Down() HeadText( "......", htx_color_0, 0, true )
  Delay_Down() HeadText( "Where's the breakfast?", htx_color_0, 0, true )
  Delay_Down() HeadText( ".........", htx_color_h, 0, true )
  Delay_Down() HeadText( "It's too late.", htx_color_h, 0, true )
  Delay_Down() HeadText( "Go to school now or you'll be late.", htx_color_h, 0, true )
  Delay_Down() HeadText( "NO.", htx_color_0, 0, true )
  Delay_Down() HeadText( "Why?", htx_color_0, 0, true )
  Delay_Down() HeadText( "You've been absent from school for several months.", htx_color_h, 0, true )
  Delay_Down() HeadText( "Go now. You will find the answer.", htx_color_h, 0, true )
  Delay_Down() HeadText( "You will see what you've been wanting.", htx_color_h, 0, true )
  Delay_Down() HeadText( "......You and your...father.", htx_color_h, 0, true )
  Delay_Down() Signal( proj, -1 )
  Delay_Down( true ) HeadText( "..........", htx_color_0, 0, true )
  Delay_Up() SetKeyInt( "$d2", 1 )
 else
  Delay_Down()
  GetMasterLevel():BlackOut( 40, 0 )
  GetCurLevel():GetPawnByName( "d" ):PlayState( "stand1" )
 end
end

function Day3_unknown_btn_4()
 local Delay_Down = function( b )
  while EvaluateKeyInt( "$btn" ) == 0 do
   coroutine.yield()
  end
  if not b then SetKeyInt( "$btn", 0 ) end
 end
 local Delay_Up = function()
  while EvaluateKeyInt( "$btn" ) == 1 do
   coroutine.yield()
  end
 end
 local player = GetPlayer()
 Delay_Down() HeadText( "Mom...", htx_color_0, 0, true )
 Delay_Down() HeadText( "I've had enough...", htx_color_0, 0, true )
 Delay_Down() HeadText( "I can't stay here anymore...", htx_color_0, 0, true )
 Delay_Down() HeadText( "I can't live like this...", htx_color_0, 0, true )
 Delay_Down() HeadText( "Like a rat slowly drowning in a sewer...", htx_color_0, 0, true )
 Delay_Down() HeadText( "I'm going to the police...", htx_color_0, 0, true )
 Delay_Down() HeadText( "I'll give myself up...", htx_color_0, 0, true )
 Delay_Down() HeadText( "I'll give everything up...", htx_color_0, 0, true )
 Delay_Down() HeadText( "I'm already done...", htx_color_0, 0, true )
 Delay_Down() HeadText( "Nothing can help...", htx_color_0, 0, true )
 Delay_Down() HeadText( "I'll end my life there...", htx_color_0, 0, true )
 Delay_Down() HeadText( "I won't miss here...", htx_color_0, 0, true )
 Delay_Down() HeadText( "I won't miss anything...", htx_color_0, 0, true )
 Delay_Down() HeadText( "Don't worry about me...", htx_color_0, 0, true )
 Delay_Down() HeadText( "Goodbye...", htx_color_0, 0, true )
 Delay_Down() HeadText( "..........", htx_color_0, 0, true )
 Delay_Down( true )
 Delay_Up() GetMasterLevel():BlackOut( 40 ) SetKeyInt( "$d1", 1 )
end

function Day3_unknown_el_door_open()
 if EvaluateKeyInt( "day3_unknown_3_el_repaired" ) > 0 or GetCurLevel():IsSnapShot() then return true end
 local player = GetPlayer()
 if player:GetEquipment( 3 ) then return false end
 return true
end

function Day3_unknown_el_target_complete( x )
 if EvaluateKeyInt( "$target" ) > 0 then return end
 GetMasterLevel():RemoveLevelMark( "m" .. tostring( x ) )
 SetKeyInt( "$target", 1 )
 local n = EvaluateKeyInt( "day3_unknown_3_quest_1" )
 SetKeyInt( "day3_unknown_3_quest_1", n + 1 )
 SetKeyInt( "day3_unknown_3_block", 1 )
end

function Day3_unknown_el_block_path()
 local player = GetPlayer()
 while not( player:GetToX() == 8 and player:GetToY() == 4 or player:GetToX() == 10 and player:GetToY() == 4 ) do coroutine.yield() end
 RunScenario( function()
  Delay( 60 )
  PlaySoundEffect( "alert1" )
  GetMasterLevel():BlackOut( 40, 0 )
  for i = 1, 7, 1 do GetCurLevel():SpawnPreset( tostring( i ) ) end
  Delay( 60 )
  
  local n = EvaluateKeyInt( "day3_unknown_3_quest_1" )
  if n == 2 then WaitFor( ScenarioDialogue( 1, "...Sorry, this way is being blocked.", dtx_color_h, -1 ), 60 )
  elseif n == 3 then WaitFor( ScenarioDialogue( 1, "Excuse me sir, your ID please.", dtx_color_h, -1 ), 60 )
  elseif n == 4 then WaitFor( ScenarioDialogue( 1, "You should not walk around at this time.", dtx_color_h, -1 ), 60 )
  end
  SetKeyInt( "$blocked", 1 )
  SetKeyInt( "day3_unknown_3_block", 0 )
  Delay( 60 )
 end )
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
  return
  --[[local dst = { ( toX + 1 ) * LEVEL_GRID_SIZE_X, ( toY + 0.5 ) * LEVEL_GRID_SIZE_Y }
  proj:SetTarget( dst )
  CreateLighningEft( proj:GetProjSrc(), dst )
  HeadText( "Library rule enforced.", htx_color_2 )
  GetMasterLevel():InterferenceStripEffect( 1, 1 )
  GetCurLevel():Fail( 0 )]]--
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
     WaitFor( ScenarioDialogue( 1, ">>>Please complete all the codes before lunchtime ends.", dtx_color_5, -1 ) )

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
     GetPlayer():PlayState( "leave" )
	 
     GetCurLevel():GetPawnByName( "sofa" ):SetLocked( true )
	 Delay( 60 )
     WaitFor( ScenarioDialogue( 0, "........................", dtx_color_1, -1, 3 ) )
	 Delay( 60 )
     GetMasterLevel():BlackOut( 10, 0 )
     WaitFor( ScenarioDialogue( 0, "........................", dtx_color_1, 60, 3 ) )
	 Delay( 30 )
     GetMasterLevel():BlackOut( 10, 0 )
     WaitFor( ScenarioDialogue( 0, "........................", dtx_color_1, 60, 3 ) )
	 Delay( 15 )
     GetMasterLevel():BlackOut( 10, 0 )
     WaitFor( ScenarioDialogue( 0, "........................", dtx_color_1, 60, 3 ) )
     GetMasterLevel():BlackOut( 30, 0 )
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
     player:PlayState( "move_up" )
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
     player:PlayState( "move_down" )
     for i = 1, 10, 1 do
      Delay( 7 )
      GetMasterLevel():BlackOut( 5, 2 )
     end
     WaitFor( ScenarioDialogue( 0, "FUCKING MAD.", dtx_color_1, 5 ) )
     Delay( 20 )
     GetMasterLevel():BlackOut( 5, 0 )
     for i0 = 1, 5, 1 do
      player:PlayStateTurnBack( "move_x" )
      for i = 1, 4, 1 do
       Delay( 8 )
	   ScenarioDialogue( 0, "FUCKING MAD.", dtx_color_1, 5 )
       GetMasterLevel():BlackOut( 5, 3 )
      end
     end
	 
     Delay( 10 )
     GetMasterLevel():BlackOut( 45, 0 )
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
       Delay( 3 )
       GetMasterLevel():BlackOut( 3, 11 - i )
       Delay( 11 - i )
      end
     end
     player:PlayStateTurnBack( "break" )
     Delay( 15 )
     GetMasterLevel():BlackOut( 180, 0 )
	 
     Delay( 180 )
     WaitFor( ScenarioDialogue( 0, "......NO......", dtx_color_1, -1 ) )
     Delay( 30 )
	 for i = 1, 4, 1 do
      GetMasterLevel():BlackOut( 10, 0 )
      Delay( 30 )
	 end
     WaitFor( ScenarioDialogue( 0, "......No, I'm not......", dtx_color_1, -1 ) )
     Delay( 45 )
	 for i = 1, 2, 1 do
      GetMasterLevel():BlackOut( 10, 0 )
      Delay( 45 )
	 end
     WaitFor( ScenarioDialogue( 0, "......I remembered something......", dtx_color_1, -1 ) )
     Delay( 60 )
	 for i = 1, 2, 1 do
      GetMasterLevel():BlackOut( 10, 0 )
      Delay( 60 )
	 end
     WaitFor( ScenarioDialogue( 0, "......I think I remember it......", dtx_color_1, -1 ) )
     Delay( 120 )
     GetCurLevel():GetPawnByName( "sofa" ):SetLocked( false )
	 SetKeyInt( "day4_secret_entry", 1 )
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
	 
     ScenarioDialogue( 0, "Steal..Theft..Robbery...No, 4 letters......", dtx_color_1, 1 )
     local pwdUI = GetMasterLevel():ShowInteractionUI( m, "data/interaction/password.pf" )
     pwdUI:SetPassword( EvaluateKeyString( "day4_pwd" ) )
     while GetMasterLevel():GetInteractionUI() do
      coroutine.yield()
     end
     GetPlayer():PlayState( "leave" )
	 
     WaitFor( ScenarioDialogue( 0, "...Must be some fucking lawyer's term.", dtx_color_1, 60, 3 ) )
     WaitFor( ScenarioDialogue( 0, "..........Fuck it.", dtx_color_1, 60, 3 ) )
	 
     SetKeyInt( "$passed", 1 )
     GetMasterLevel():CheckPoint()
     GetMasterLevel():BlackOut( 30, 0 )
	end )

   elseif CurTime() >= 3 and CurTime() <= 4 then
    if not FEVT( "$cheatproof" ) then
	 HeadText( "You fucking cheater.", htx_color_h )
	 GetCurLevel():Fail( 1 )
	 return
	end
    SetKeyString( "day4_pwd", "SHIT" )
    RunScenario( function()
     Delay( 60 )
     WaitFor( ScenarioDialogue( 1, "........OK.", dtx_color_5, 60 ) )
     ScenarioDialogue( 1, ">>>CODE 3: Please input the lunch today.", dtx_color_5, -1 )
     local pwdUI = GetMasterLevel():ShowInteractionUI( m, "data/interaction/password.pf" )
     pwdUI:SetPassword( EvaluateKeyString( "day4_pwd" ) )
     while GetMasterLevel():GetInteractionUI() do
      coroutine.yield()
     end
     GetPlayer():PlayState( "leave" )
	 
     WaitFor( ScenarioDialogue( 0, ".........", dtx_color_1, 60, 6 ) )
     WaitFor( ScenarioDialogue( 0, "...MASTER.", dtx_color_1, 60, 3 ) )
     WaitFor( ScenarioDialogue( 0, "...MASTER. Are you there?", dtx_color_1, 60, 3 ) )
     WaitFor( ScenarioDialogue( 0, ".........", dtx_color_1, -1, 6 ) )
	 Delay( 60 )
	 
     SetKeyInt( "$passed", 1 )
     GetMasterLevel():CheckPoint()
     GetMasterLevel():BlackOut( 30, 0 )
	end )

   elseif CurTime() == 5 then
    RunScenario( function()
     Delay( 60 )
     WaitFor( ScenarioDialogue( 1, "........OK.", dtx_color_5, 60 ) )
     WaitFor( ScenarioDialogue( 0, ".........", dtx_color_1, 60, 6 ) )
	 Delay( 120 )
     WaitFor( ScenarioDialogue( 1, "OK those trivial procedural works are finally over.", dtx_color_h, 100 ) )
     WaitFor( ScenarioDialogue( 1, "You've wasted too much time.", dtx_color_h, 60 ) )
     WaitFor( ScenarioDialogue( 1, "Now we must go to the final step.", dtx_color_h, -1 ) )
	 Delay( 60 )
     WaitFor( ScenarioDialogue( 0, ".........", dtx_color_1, 60, 6 ) )
	 Delay( 60 )
     WaitFor( ScenarioDialogue( 1, "...According to the system log,", dtx_color_h, 70 ) )
     WaitFor( ScenarioDialogue( 1, "Before the moment you were deprived of the admin permission,", dtx_color_h, 120 ) )
     WaitFor( ScenarioDialogue( 1, "You were trying to submit a report,", dtx_color_h, 90 ) )
     WaitFor( ScenarioDialogue( 1, "But the procedure was somehow interrupted,", dtx_color_h, 100 ) )
     WaitFor( ScenarioDialogue( 1, "Leaving the transaction incomplete.", dtx_color_h, 90 ) )
     WaitFor( ScenarioDialogue( 1, "Before we can normally recover your permission,", dtx_color_h, 110 ) )
     WaitFor( ScenarioDialogue( 1, "We must clean this up first.", dtx_color_h, -1 ) )
     WaitFor( ScenarioDialogue( 0, ".........", dtx_color_1, -1 ) )
     WaitFor( ScenarioDialogue( 1, "There are some complex technical details inside.", dtx_color_h, 110 ) )
     WaitFor( ScenarioDialogue( 1, "To put simply, the report needs to be verified and approved.", dtx_color_h, 130 ) )
     WaitFor( ScenarioDialogue( 1, "But there are some errors preventing it from being accessed normally.", dtx_color_h, 160 ) )
     WaitFor( ScenarioDialogue( 1, "You referred to someone in it. But the pointer,", dtx_color_h, 110 ) )
     WaitFor( ScenarioDialogue( 1, "Which should indicate the one you alleged *abnormal*,", dtx_color_h, 110 ) )
     WaitFor( ScenarioDialogue( 1, "Now seems invalid.", dtx_color_h, -1 ) )
     WaitFor( ScenarioDialogue( 0, ".........", dtx_color_1, -1 ) )
     WaitFor( ScenarioDialogue( 1, "There's a new job for you.", dtx_color_h, 70 ) )
     WaitFor( ScenarioDialogue( 1, "You have some time to rest and get prepared. No need to hurry.", dtx_color_h, 120 ) )
     WaitFor( ScenarioDialogue( 1, "When you're ready, set off and go to a console.", dtx_color_h, 100 ) )
     WaitFor( ScenarioDialogue( 1, "Not a hard job. You just need to follow my instruction.", dtx_color_h, 110 ) )
	 Delay( 80 )
	 
     SetKeyInt( "$passed", 1 )
     GetMasterLevel():CheckPoint()
	end )
    
   else
    GetPlayer():PlayState( "leave" )
   end
  elseif nSig == 2 then
  elseif nSig == 3 then
  end
  nSig = coroutine.yield( 1 )
 end
end

function Day4_End_1()
 Delay( 60 )
 WaitFor( ScenarioDialogue( 0, "..........", dtx_color_1, -1, 6 ), 60 )
 Delay( 40 )
 WaitFor( ScenarioDialogue( 1, "..........", dtx_color_h, -1, 6 ), 60 )
 Delay( 80 )
 GetMasterLevel():BlackOut( 60, 0 )
 Delay( 30 )
 WaitFor( ScenarioDialogue( 0, "............", dtx_color_1, -1, 6 ), 60 )
 WaitFor( ScenarioDialogue( 1, "........................", dtx_color_h, -1, 6 ), 120 )
 Delay( 60 )
 GetMasterLevel():BlackOut( 60, 0 )
 Delay( 120 )
 GetMasterLevel():BlackOut( 60, 0 )
 Delay( 120 )
 WaitFor( ScenarioDialogue( 1, "........................", dtx_color_h, -1, 12 ), 120 )
 Delay( 40 )
 for i = 1, 4, 1 do
  GetMasterLevel():BlackOut( 30, 0 )
  Delay( 30 )
 end
 WaitFor( ScenarioDialogue( 0, "............", dtx_color_1, -1, 6 ), 60 )
 WaitFor( ScenarioDialogue( 0, "1fif398 ds9h0bre", dtx_color_5, -1 ), 60 )
 WaitFor( ScenarioDialogue( 1, "........................", dtx_color_h, -1, 6 ), 120 )
 WaitFor( ScenarioDialogue( 0, "oveni peij9e3v99 98q3bo asd908y9daawf ps9uv9ps", dtx_color_5, -1 ), 60 )
 Delay( 10 )
 PlaySoundEffect( "bzzz0" )
 GetMasterLevel():BlackOut( 10, 0 )
 WaitFor( ScenarioDialogue( 0, "sdfweoiv &*|*9fds *DF&9Df9 (*&f", dtx_color_5, -1 ), 60 )
 Delay( 10 )
 PlaySoundEffect( "bzzz0" )
 GetMasterLevel():BlackOut( 10, 0 )
 WaitFor( ScenarioDialogue( 0, "v093q2(*Y*Fpo)P(SDUF]{SDF] (SDDSF9 (*&(*)*&#&*!^31^31^31^31^31^31^31^31^31", dtx_color_5, -1 ), 60 )
 for i = 1, 3, 1 do
  Delay( 10 )
  PlaySoundEffect( "bzzz0" )
  GetMasterLevel():BlackOut( 10, 0 )
 end
 WaitFor( ScenarioDialogue( 0, "^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31", dtx_color_5, -1 ), 120 )
 for i = 1, 3, 1 do
  Delay( 5 )
  PlaySoundEffect( "bzzz0" )
  GetMasterLevel():BlackOut( 5, 0 )
 end
 Delay( 45 )
 for i = 1, 8, 1 do
  PlaySoundEffect( "bzzz0" )
  GetMasterLevel():BlackOut( 5, 0 )
  Delay( 5 )
 end
 GetMasterLevel():BlackOut( 60, 0 )
 Delay( 20 )
 WaitFor( ScenarioDialogue( 1, "......Are you sure your input is correct?", dtx_color_h, -1, 2 ), 60 )
 WaitFor( ScenarioDialogue( 0, "......YES certainly......", dtx_color_1, -1, 2 ) )
 WaitFor( ScenarioDialogue( 0, "There has to be no problem......", dtx_color_1, -1 ), 60 )
 WaitFor( ScenarioDialogue( 1, "......I'm afraid you shouldn't be that confident.", dtx_color_h, -1 ), 60 )
 WaitFor( ScenarioDialogue( 1, "That room is empty. I can make sure.", dtx_color_h, -1, 2 ), 60 )
 WaitFor( ScenarioDialogue( 0, "No that's impossible...it was me...", dtx_color_1, -1, 2 ) )
 WaitFor( ScenarioDialogue( 1, "Yes there is just nobody. See it on your own.", dtx_color_h, -1, 2 ), 60 )
 Delay( 60 )
 for i = 1, 10, 1 do
  GetMasterLevel():GetMainUI():ShowFreezeEft( i )
  Delay( 6 )
 end
 TransferTo( "stages/5f_east_build_c_103_2.pf", 2, 0, 0, -2 )
end

function Day4_End_2()
 local player = GetPlayer()
 Delay( 60 )
 GetMasterLevel():BlackOut( 5, 0 )
 player:SetForceHide( true )
 Delay( 60 )
 GetMasterLevel():BlackOut( 5, 0 )
 player:SetForceHide( false )
 Delay( 40 )
 GetMasterLevel():BlackOut( 5, 0 )
 player:SetForceHide( true )
 Delay( 40 )
 GetMasterLevel():BlackOut( 5, 0 )
 player:SetForceHide( false )
 for i = 1, 2, 1 do
  Delay( 30 )
  GetMasterLevel():BlackOut( 5, 0 )
  player:SetForceHide( true )
  Delay( 30 )
  GetMasterLevel():BlackOut( 5, 0 )
  player:SetForceHide( false )
 end
 for i = 1, 4, 1 do
  Delay( 20 )
  GetMasterLevel():BlackOut( 5, 0 )
  player:SetForceHide( true )
  Delay( 20 )
  GetMasterLevel():BlackOut( 5, 0 )
  player:SetForceHide( false )
 end
 for i = 1, 10, 1 do
  Delay( 10 )
  GetMasterLevel():BlackOut( 5, 0 )
  player:SetForceHide( true )
  Delay( 10 )
  GetMasterLevel():BlackOut( 5, 0 )
  player:SetForceHide( false )
 end
 Delay( 20 )
 GetMasterLevel():BlackOut( 90, 0 )
 player:SetForceHide( true )
 Delay( 120 )
 
 WaitFor( ScenarioDialogue( 0, "..........", dtx_color_1, -1, 6 ) )
 WaitFor( ScenarioDialogue( 0, "...No how did this...", dtx_color_1, -1, 6 ), 60 )
 WaitFor( ScenarioDialogue( 1, "That's just it. Nobody is there.", dtx_color_h, 80 ) )
 WaitFor( ScenarioDialogue( 1, "If you're going to say someone was there and then escaped......", dtx_color_h, 150 ) )
 Delay( 60 )
 WaitFor( ScenarioDialogue( 1, "......I'm afraid I would understand.", dtx_color_h, -1, 6 ) )
 Delay( 120 )
 WaitFor( ScenarioDialogue( 0, "...What did you...", dtx_color_1, -1 ) )
 WaitFor( ScenarioDialogue( 1, "I mean we must move to plan B.", dtx_color_h, -1, 6 ) )
 Delay( 120 )
 WaitFor( ScenarioDialogue( 1, "It's too late now. Let's continue tomorrow.", dtx_color_h, 100 ) )
 WaitFor( ScenarioDialogue( 1, "For now you have to know only one thing.", dtx_color_h, 100 ) )
 WaitFor( ScenarioDialogue( 1, "We're fucking having big troubles.", dtx_color_h, 100, 4 ) )
 Delay( 60 )
 WaitFor( ScenarioDialogue( 1, "REALLY BIG TROUBLES.", dtx_color_h, 100, 6 ) )
 Delay( 60 )
 WaitFor( ScenarioDialogue( 1, "BECAUSE OF YOU.", dtx_color_h, -1, 6 ) )

 Delay( 180 )
 GetMasterLevel():BlackOut( 100, 180 )
 --TransferTo( "data/cutscene/day5.pf" )
 TransferTo( "data/cutscene/end.pf" )
end

function Day4_Secret_Scene_1()
 Delay( 1 )
 local player = GetPlayer()
 local screen = GetCurLevel():GetPawnByName( "tv" ):FindChildEntity( "screen" )
 local start = screen:FindChildEntity( "start" )
 local typing = screen:FindChildEntity( "typing" )
 local red = screen:FindChildEntity( "red" )

 local text = { '"TIME HAS PASSED SINCE EVERYTHING BACK TO ORDER AFTER THE REVOLUTION..."',
  '"THE ORDER NEVER RETURNED TO THIS LAND..."',
  '"EVIL IS STILL HIDING BENEATH THE EARTH...FLOATING IN THE AIR...LOOMING IN THE SUNLIGHT..."',
  '"MADNESS IS WAITING FOR THE DAY WHEN THE SEAL IS BROKEN..."',
  '"ON THAT DAY YOU WAS STAYING AT HOME THE WHOLE DAYTIME..."',
  '"THE NIGHT FINALLY CAME DOWN...YOU REALIZED YOUR FOOD IS RUNNING OUT..."',
  '"WHAT A HORRIBLE NIGHT TO HAVE A CURSE..."', }
 local chars = { "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "A", "B", "C", "D", "E", "F" }

 if EvaluateKeyInt( "unknown_d4_video_secret_room_progress" ) > 0 then
  screen:SetVisible( true )
  screen:FindChildEntity( "a" ):SetVisible( false )
  start:SetVisible( false )
  typing:SetVisible( false )
  red:SetVisible( true )
  goto ai_end
 end

 while EvaluateKeyInt( "$sofa" ) == 0 do Delay( 1 ) end
 screen:SetVisible( true )
 start:SetVisible( false )
 typing:SetVisible( false )
 red:SetVisible( false )
 Delay( 50 )
 screen:FindChildEntity( "a" ):SetVisible( false )
 start:SetVisible( true )
 while EvaluateKeyInt( "$press_a" ) == 0 do Delay( 1 ) end
 SetKeyInt( "$press_a", 0 )
 start:SetVisible( false )
 typing:SetVisible( true )

 for i = 1, #text, 1 do
  HeadText( text[i], htx_color_5 )
  local str = ""
  local n = 0
  while EvaluateKeyInt( "$press_a" ) == 0 do
   if #str < 25 then
    if n == 0 then
     str = str .. chars[RandInt( 1, 17 )]
	 typing:Set( str )
     n = 3
    end
    n = n - 1
   end
   Delay( 1 )
  end
  SetKeyInt( "$press_a", 0 )
 end
 typing:SetVisible( false )
 red:SetVisible( true )

 player:PlayState( "s1", 1 )
 while EvaluateKeyInt( "$sofa" ) == 1 do Delay( 1 ) end
 SetKeyInt( "unknown_d4_video_secret_room_progress", 1 )
::ai_end::
 for i = 1, 4, 1 do
  local p = GetCurLevel():GetPawnByName( tostring( i ) )
  if p then p:SetLocked( false ) end
 end
 Signal( GetCurLevel():GetPawnByName( "ai_2" ), 0 )
end

function Day4_Secret_Scene_2()
 while EvaluateKeyInt( "$sofa" ) == 0 do Delay( 1 ) end
 Delay( 40 )
 HeadText( '"WHAT ARE YOU GOING TO DO?"', htx_color_5 )
 while EvaluateKeyInt( "$press_a" ) == 0 and EvaluateKeyInt( "$press_b" ) == 0 and EvaluateKeyInt( "$press_c" ) == 0
  and EvaluateKeyInt( "$sofa" ) == 1 do Delay( 1 ) end
 SetKeyInt( "$press_a", 0 ) SetKeyInt( "$press_b", 0 ) SetKeyInt( "$press_c", 0 )
 if EvaluateKeyInt( "$sofa" ) == 0 then return end
 
 local light_switch = { desc = "LIGHT SWITCH", opr = function()
  PlaySoundEffect( "btn" )
  SetKeyInt( "$light", 1 - EvaluateKeyInt( "$light" ) )
 end }
 local door_switch = { desc = "DOOR SWITCH", opr = function()
  SetKeyInt( "$d0", 1 - EvaluateKeyInt( "$d0" ) )
 end }

 local states = {
  {
   { desc = "???", opr = function() end },
   light_switch,
   door_switch
  }

 }

 local curState = states[1]
 while true do
  local strDesc = '"'
  local prefix = { "A -- ", "B -- ", "C -- " }
  local key = { "$press_a", "$press_b", "$press_c" }
  for i = 1, 3, 1 do
   if curState[i].desc then
    strDesc = strDesc .. prefix[i] .. curState[i].desc .. "   "
   end
  end
  strDesc = strDesc .. 'D -- LEAVE"'
  HeadText( strDesc, htx_color_5 )

  while true do
   if EvaluateKeyInt( "$sofa" ) == 0 then HeadText( "" ) return end
   for i = 1, 3, 1 do
    if EvaluateKeyInt( key[i] ) > 0 then
	 SetKeyInt( key[i], 0 )
	 local ret = curState[i].opr()
	 if ret then curState = states[ret] end
	 break
	end
   end
   Delay( 1 )
  end

 end
end

function Day4_Secret_Scene_Err()
 Delay( 60 )
 local nLoad = 0
 for i = 1, 4, 1 do
  if EvaluateKeyInt( "%day4_secret_texture_" .. tostring( i ) ) == 0 then
   nLoad = i
   break
  end
 end
 if nLoad > 0 then
  local tbl = { htx_color_5, htx_color_sys, htx_color_2, htx_color_1 }
  HeadText( "Loading assets...", tbl[nLoad] )
  Delay( 300 )
  while true do
   HeadText( "Error: Texture block damaged. Trying to restore...", tbl[nLoad] )
   Delay( 300 )
  end
 end
 while true do Delay( 1 ) end
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
 WaitFor( ScenarioDialogue( 0, "Shit shit shit..", dtx_color_1, 120 ), ScenarioWaitInput() )
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
 WaitFor( ScenarioDialogue( 1, "You are fucking calling me the thief, you blind crook?", dtx_color_6b, 60 ) )
 WaitFor( ScenarioDialogue( 1, "Who will give a shit to that crap?", dtx_color_6b, 60 ) )
 WaitFor( ScenarioDialogue( 1, "Anyone not as fucking blind as you?", dtx_color_6b, 60 ) )
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
  HeadText( "Fuck, fuck...", htx_color_6b, 0, true )
  Delay( 180, 1, 1 )
  PlaySoundEffect( "door_hit" )
  Delay( 60 )
  HeadText( "Stop it idiot.", htx_color_6b, 0, true )
  Delay( 120, 0, 0 )
  PlaySoundEffect( "door_hit" )
  Delay( 60 )
  HeadText( "Useless.", htx_color_6b, 0, true )
  Delay( 120 )
  Delay( 60, 2, 1 )
  PlaySoundEffect( "door_hit" )
  Delay( 60 )
  HeadText( "...Fuck it went wrong again.", htx_color_6b, 0, true )
  Delay( 60 )
  PlaySoundEffect( "door_hit" )
  Delay( 60 )
  HeadText( "Reset it mecha-jackass. Are you fucking blind too?", htx_color_6b, 0, true )
  for i = 1, 4, 1 do
   Delay( 20, 0, ( i - 1 ) % 2 )
  end
  Delay( 90 )
  HeadText( "Did your creator plug into the wrong hole?", htx_color_6b, 0, true )
  Delay( 150 )
  HeadText( "..........", htx_color_6b )
  Delay( 90 )
  Delay( 60, 2, 1 )
  HeadText( "Hi Kucha. Please reset.", htx_color_6b, 0, true )
  local function ResetSecurity()
   Delay( 90 )
   HeadText( "Security system reseting...", htx_color_sys, 120, true )
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
  HeadText( "Fuck you. Are't you going to rest for a fucking while?", htx_color_6b, 0, true )
  Delay( 120, 0, 0 )
  HeadText( "...What the fuck did you say?", htx_color_6b, 0, true )
  Delay( 120, 3, 0 )
  HeadText( "...Stop fucking me. Go fuck Lawyer " .. NAME_LAWYER .. ".", htx_color_6b, 0, true )
  Delay( 120, 3, 0 )
  HeadText( "He's the one who fucked us all.", htx_color_6b, 0, true )
  Delay( 100 )
  HeadText( "He told us to do as he said.", htx_color_6b, 0, true )
  Delay( 90 )
  HeadText( "He made our sentences.", htx_color_6b, 0, true )
  Delay( 80 )
  HeadText( "He packed us together to be sent here.", htx_color_6b, 0, true )
  Delay( 80 )
  HeadText( "Why don't you go fuck his guts out of his asshole for us?", htx_color_6b, 0, true )
  Delay( 150 )
  HeadText( "......What?", htx_color_6b, 0, true )
  bKnock = false
  Delay( 150 )
  HeadText( "......Say it again?", htx_color_6b, 0, true )
  SetKeyInt( "4f_center_room_2a_d1", 0 )
  Delay( 120, 1, 0 )
  for i = 1, 2, 1 do
   LevelRegisterUpdate1( function() GetCurLevel():SpawnPawn1( "data/enemies/enemy_attack_0.pf", pawn:GetToX(), pawn:GetToY(), 1 ) end )
   Delay( 60, 4, 0 )
  end
  HeadText( "...Oh fuck...", htx_color_6b, 0, true )
  Delay( 120 )
  for i = 1, 4, 1 do
   LevelRegisterUpdate1( function() GetCurLevel():SpawnPawn1( "data/enemies/enemy_attack_0.pf", pawn:GetToX(), pawn:GetToY(), 1 ) end )
   Delay( 50, 4, 0 )
  end
  Delay( 40, 1, 1 )
  HeadText( "...Damn it...", htx_color_6b, 0, true )
  Delay( 60 )

  if GetCurLevel():FindChildEntity( "tutorial_follow" ):IsAnythingAbnormal() then
   HeadText( "...Screw this...What's fucking wrong with it today?", htx_color_6b, 0, true )
   Delay( 120 )
   HeadText( "...Kucha, reset.", htx_color_6b, 0, true )
   LevelRegisterUpdate1( ResetSecurity )
   Delay( 240 )
  else
   Delay( 40 )
   HeadText( "...This machine better not get me in trouble.", htx_color_6b, 0, true )
   Delay( 120 )
  end

  Delay( 40, 0, 0 )
  HeadText( "...Come in, motherfucker. Get inside.", htx_color_6b, 0, true )
  Delay( 100 )
  HeadText( "Don't just stand there. Get in and enjoy yourself.", htx_color_6b, 0, true )
  Delay( 100 )
  HeadText( "We can watch some cute videos together.", htx_color_6b, 0, true )
  Delay( 100 )
  HeadText( "Or what about your favorite colletion?", htx_color_6b, 0, true )
  Delay( 100 )
  HeadText( "..........", htx_color_6b, 0, true )
  Delay( 100 )
  HeadText( "...Dare you not.", htx_color_6b, 0, true )
  Delay( 140 )
  HeadText( "...Ok. So where were we? Did you remember?", htx_color_6b, 0, true )
  Delay( 100 )
  HeadText( "...Yes, that's it. I know it. You are fucking right.", htx_color_6b, 0, true )
  Delay( 120 )
  HeadText( "...Yes yes yes. Don't remind me. I really fucking know it.", htx_color_6b, 0, true )
  Delay( 120 )
  HeadText( "...Very good. That's the very fucking point. I loved her.", htx_color_6b, 0, true )
  Delay( 120 )
  HeadText( "...While her parents didn't. Isn't it fucking fair?", htx_color_6b, 0, true )
  Delay( 120 )
  HeadText( "...Yes yes. Sick enough to love her.", htx_color_6b, 0, true )
  Delay( 120 )
  HeadText( "Sick enough to protect her from being used as a body shield.", htx_color_6b, 0, true )
  Delay( 120 )
  HeadText( "I'm a fucking pervert. A fucking sex freak. Any questions?", htx_color_6b, 0, true )
  Delay( 150 )
  HeadText( "...Then fuck off. We have nothing to say anymore.", htx_color_6b, 0, true )
  Delay( 100 )
  SetKeyInt( "4f_center_room_2a_d1", 1 )
  HeadText( "...Fuck you. I'm not listening. Question time is over.", htx_color_6b, 0, true )
  Delay( 60 )
  PlaySoundEffect( "door_hit" )
  Delay( 30 )
  Delay( 60, 2, 1 )
  HeadText( "...I'm watching TV. I'm not listening to your bullshit.", htx_color_6b, 0, true )
  Delay( 60 )
  GetCurLevel():GetPawnByName( "tv" ):FindChildEntity( "screen" ):SetVisible( true )
  GetCurLevel():BeginNoise( "noise_tv" )
  Delay( 30 )
  PlaySoundEffect( "door_hit" )

  Delay( 60 )
  HeadText( "...I'm NOT LISTENING. Whatever you say.", htx_color_6b, 0, true )
  Delay( 90 )
  HeadText( '"UPCOMING: Mystery of the origin of Yellow August"', htx_color_5, 0, true )
  Delay( 110 )
  HeadText( '"FROM -- AUTO RECOMMENDATION"', htx_color_5, 0, true )
  Delay( 80 )
  PlaySoundEffect( "door_hit" )
  Delay( 30 )
  HeadText( "...I'm NOT FUCKING LISTENING.", htx_color_6b, 0, true )
  Delay( 60 )
  HeadText( '"...As the most decisive event in the second half of the last century..."', htx_color_5, 0, true )
  Delay( 200 )
  HeadText( '"...The whole country is burning..^31^31Prisons are overflowing...^31^31^31"', htx_color_5, 0, true )
  Delay( 200 )
  HeadText( "[Lots of noise and glitch]", htx_color_5, 0, true )
  Delay( 90 )
  HeadText( "..........", htx_color_6b, 0, true )
  Delay( 60 )
  HeadText( '"...About the trigger for the crisis there are many controversies^31^31"', htx_color_5, 0, true )
  Delay( 200 )
  HeadText( "^31^31^31^31^31^31^31^31", htx_color_5, 0, true )
  Delay( 90 )
  HeadText( '"...He said he witnessed..."', htx_color_5 )
  Delay( 100 )
  HeadText( "^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31", htx_color_5, 0, true )
  Delay( 60 )
  HeadText( '"^31^31^31^31as among the protest and was surrounded by people..^31^31^31^31"', htx_color_5, 0, true )
  Delay( 150 )
  HeadText( '"^31^31^31parade stopp^31^31^31^31 saw they formed two walls^31^31^31^31^31^31"', htx_color_5, 0, true )
  Delay( 150 )
  HeadText( "...NOT FUCKING LISTENING.", htx_color_6b, 0, true )
  Delay( 60 )
  HeadText( '"...Someone was arr^31sted^31^31^31^31^31^31e girl was sit^31ing th^31re cryin^31^31^31^31^31^31^31^31^31"', htx_color_5, 0, true )
  Delay( 150 )
  HeadText( '"^31^31^31^31^31 wa^31^31ed aw^31^31 q^31ickl^31^31^31^31^31^31So^31n I h^31ar^31^31^31^31^31^31eaming^31^31^31^31^31^31"', htx_color_5, 0, true )
  Delay( 150 )
  HeadText( "[The rest became totally illegible]", htx_color_5, 0, true )
  Delay( 90 )
  
  if GetCurLevel():FindChildEntity( "tutorial_follow" ):IsAnythingAbnormal() then
   HeadText( "NOT......What the hell is this piece of scrap doing?", htx_color_6b, 0, true )
   Delay( 100 )
   HeadText( "...Kucha, SHUTDOWN.", htx_color_6b, 0, true )
   Delay( 100 )
   HeadText( "ERROR: inadequate permissions.", htx_color_1, 0, true )
   Delay( 100 )
   HeadText( "THEN FUCKING RESET.", htx_color_6b, 0, true )
   Delay( 100 )
   HeadText( "...Reset, I mean.", htx_color_6b, 0, true )
   LevelRegisterUpdate1( ResetSecurity )
   Delay( 240 )
   HeadText( "...Enough. I've had fucking enough.", htx_color_6b, 0, true )
  else
   HeadText( "NOT...Enough.", htx_color_6b, 0, true )
   Delay( 120 )
   HeadText( "I've had enough.", htx_color_6b, 0, true )
  end
  Delay( 45 )
  GetCurLevel():GetPawnByName( "tv" ):FindChildEntity( "screen" ):SetVisible( false )
  GetCurLevel():EndNoise( "noise_tv" )
  Delay( 45 )
  Delay( 120, 3, 0 )
  HeadText( "...Fucking gone finally.", htx_color_6b, 0, true )

  context.bFin = true
 end )

 local FuncLoop = function( name, ... )
  if not GetPlayer():IsHidden() or ai:IsSeePlayer() then
   HeadText( "...What's that?", dtx_color_6b, 240 )
   SetKeyInt( "4f_center_room_2a_d1", 1 )
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

function Day4_4_Unknown_Password()
 RunScenario( function()
  local n = EvaluateKeyInt( "$n" )
  if n >= 1 then
   local pawn = GetCurLevel():GetPawnByName( tostring( n ) )
   local proj = GetCurLevel():FindChildEntity("proj")
   local src = proj:GetProjSrc()
   proj:Follow( pawn )
 
   for i = 1, 6, 1 do
    local dst = { ( pawn:GetPosX() + 1 ) * LEVEL_GRID_SIZE_X, pawn:GetPosY() * LEVEL_GRID_SIZE_Y + 48 }
    CreateLighningEft( src, dst, 1, 0.125, 40 )
    pawn:ScriptDamage( 1 )
    PlaySoundEffect( "electric" )
    Delay( 30 )
   end
   proj:Follow( nil )

   Delay( 80 )
   if n == 4 then
    ScenarioDialogue( 0, "........", dtx_color_1, 60, 3 )
    Delay( 180 )
    GetMasterLevel():BlackOut( 60, 10 )
    Delay( 5 )
    pawn:PlayState( "stand_0" )
    Delay( 180 )
   
    WaitFor( ScenarioDialogue( 0, "You are not a fucking lawyer.", dtx_color_1, 60, 3 ), 120 )
    WaitFor( ScenarioDialogue( 0, "I want my lawyer.", dtx_color_1, 60, 3 ), 120 )
    WaitFor( ScenarioDialogue( 0, "Where is he?", dtx_color_1, 60, 3 ), 120 )
    Delay( 120 )
    GetMasterLevel():BlackOut( 30, 10 )
    Delay( 5 )
    GetCurLevel():RemovePawn( pawn )
    Delay( 20 )
    SetKeyInt( "$passed", 1 )
    Delay( 180 )
    GetCurLevel():GetPawnByName( "sofa" ):SetLocked( true )
    GetCurLevel():GetPawnByName( "medikit" ):SetLocked( false )
	return
   else
    GetMasterLevel():BlackOut( 30, 10 )
    Delay( 5 )
    GetCurLevel():RemovePawn( pawn )
   end

  end

  n = n + 1
  SetKeyInt( "$n", n )
  Delay( 45 )
  local pwdUI = nil

  if n == 1 then
   WaitFor( ScenarioDialogue( 1, ">>>CODE 3: Please input the lunch today.", dtx_color_5, -1 ), 90 )
   ScenarioDialogue( 0, "Shit..Eat shit you motherfucking code...", dtx_color_1, 1 )
   pwdUI = GetMasterLevel():ShowInteractionUI( m, "data/interaction/password.pf" )
   pwdUI:SetPassword( "S" )
  elseif n == 2 then
   WaitFor( ScenarioDialogue( 0, "SHIT...I knew...", dtx_color_1, -1 ), 60 )
   Delay( 60 )
   pwdUI = GetMasterLevel():ShowInteractionUI( m, "data/interaction/password.pf" )
   pwdUI:SetPassword( "SH" )
  elseif n == 3 then
   WaitFor( ScenarioDialogue( 0, "Shit shit shit...", dtx_color_1, -1 ), 60 )
   Delay( 60 )
   pwdUI = GetMasterLevel():ShowInteractionUI( m, "data/interaction/password.pf" )
   pwdUI:SetPassword( "SHI" )
  elseif n == 4 then
   WaitFor( ScenarioDialogue( 0, "SHITSHITSHITSHIT...", dtx_color_1, -1 ), 60 )
   Delay( 60 )
   for i = 1, 4, 1 do
    GetMasterLevel():BlackOut( 10, 0 )
    Delay( 30 )
   end
   pwdUI = GetMasterLevel():ShowInteractionUI( m, "data/interaction/password.pf" )
   pwdUI:SetPassword( "SHIT" )
  end

  while GetMasterLevel():GetInteractionUI() do
   coroutine.yield()
  end
  GetMasterLevel():BlackOut( 60, 0 )
  
  if n == 1 then
   WaitFor( ScenarioDialogue( 1, ">>>ERROR: The verification code is out of date.", dtx_color_3, -1 ), 90 )
   GetMasterLevel():BlackOut( 10, 0 )
   Delay( 80 )
   GetMasterLevel():BlackOut( 10, 0 )
   ScenarioDialogue( 0, "No no shit...", dtx_color_1, 120 )
   Delay( 80 )
   for i = 1, 3, 1 do
    GetMasterLevel():BlackOut( 10, 0 )
    Delay( 80 )
   end
   GetMasterLevel():BlackOut( 60, 0 )

   for i = 1, 8, 1 do
    PlaySoundEffect( "footstep_a" )
    Delay( 80 )
   end
   GetMasterLevel():BlackOut( 10, 0 )
   Delay( 80 )
   GetMasterLevel():BlackOut( 10, 0 )
   Delay( 80 )
   GetMasterLevel():BlackOut( 40, 10 )
   PlaySoundEffect( "glass_break" )
   GetCurLevel():GetPawnByName( "tv" ):FindChildEntity( "broken" ):SetVisible( true )
   Delay( 5 )
   GetCurLevel():SpawnPreset( "g0" )
   GetCurLevel():SpawnPreset( "g1" )
   GetCurLevel():SpawnPreset( "p" )
   Delay( 80 )
   WaitFor( ScenarioDialogue( 1, "...........", dtx_color_3, 60, 6 ), 120 )
   Delay( 60 )
  elseif n == 2 then
   WaitFor( ScenarioDialogue( 1, ">>>ERROR: Unable to call handle function. Module uninstalled.", dtx_color_3, -1 ), 120 )
   GetMasterLevel():BlackOut( 10, 0 )
   Delay( 80 )
   GetMasterLevel():BlackOut( 10, 0 )
   Signal( GetCurLevel():GetPawnByName( "medikit" ), 0 )
   Delay( 80 )
  elseif n == 3 then
   WaitFor( ScenarioDialogue( 1, ">>>ERROR: Device integration damaged. Further exec stopped.", dtx_color_3, -1 ), 120 )
   GetMasterLevel():BlackOut( 10, 0 )
   Delay( 80 )
   GetMasterLevel():BlackOut( 10, 0 )
   Signal( GetCurLevel():GetPawnByName( "medikit" ), 0 )
   Delay( 80 )
  elseif n == 4 then
   WaitFor( ScenarioDialogue( 1, ">>>Please contact our MASTER for help.", dtx_color_3, -1 ), 150 )
   for i = 1, 4, 1 do
    GetMasterLevel():BlackOut( 10, 0 )
    Delay( 40 )
   end
   WaitFor( ScenarioDialogue( 0, "I'm done...", dtx_color_1, 60 ), 60 )
   GetMasterLevel():BlackOut( 20, 0 )
   Delay( 60 )
   GetMasterLevel():BlackOut( 20, 0 )
   Delay( 60 )
   WaitFor( ScenarioDialogue( 0, "All is done...", dtx_color_1, 60 ), 60 )
   GetMasterLevel():BlackOut( 20, 0 )
   Delay( 60 )
   GetMasterLevel():BlackOut( 20, 0 )
   Signal( GetCurLevel():GetPawnByName( "medikit" ), 0 )
   Delay( 60 )
  end

  SetKeyInt( "$passed", 1 )
  Delay( 50 )
  GetMasterLevel():BlackOut( 30, 10 )
  Delay( 5 )
  GetCurLevel():SpawnPreset( tostring( n ) )
  Delay( 80 )
  SetKeyInt( "$passed", 0 )

  if n == 1 then
   WaitFor( ScenarioDialogue( 1, "...........", dtx_color_6, 60, 3 ), 120 )
   WaitFor( ScenarioDialogue( 1, "Hello?", dtx_color_6, 60 ), 80 )
  elseif n == 2 then
   WaitFor( ScenarioDialogue( 1, "It's me. Your lawyer.", dtx_color_6, 60 ), 120 )
   WaitFor( ScenarioDialogue( 1, "We can have a talk.", dtx_color_6, 60 ), 80 )
  elseif n == 3 then
   WaitFor( ScenarioDialogue( 1, "You should try to face it.", dtx_color_6, 60 ), 120 )
   WaitFor( ScenarioDialogue( 1, "Isn't it what you want?", dtx_color_6, 60 ), 80 )
  elseif n == 4 then
   WaitFor( ScenarioDialogue( 1, "OK it's time now.", dtx_color_6, 60 ), 120 )
   WaitFor( ScenarioDialogue( 1, "You won't have to worry anymore.", dtx_color_6, 60 ), 80 )
  end
  
  Delay( 10 )
  GetMasterLevel():BlackOut( 10, 0 )
  Delay( 10 )
  WaitFor( ScenarioDialogue( 0, "........", dtx_color_1, 60, 3 ), 80 )
  Delay( 60 )
 end )
end

function Day4_4_Unknown_Medikit()
 local player = GetPlayer()
 while true do
  coroutine.yield()
  local str = EvaluateKeyString( "$m_state" )
  if #str > 0 then
   SetKeyString( "$m_state", "" )
   Delay( 2 )
   if string.sub( str, 1, 6 ) == "attack" then
	player:PlayState( "break", 1 )
    GetMasterLevel():BlackOut( 10, 10 )
   else
    GetMasterLevel():BlackOut( 30, 50 )
   end
  end
 end
end

function Day4_4_Unknown_Toilet()
 local Delay_Down = function( b )
  while EvaluateKeyInt( "$btn" ) == 0 do
   coroutine.yield()
  end
  if not b then SetKeyInt( "$btn", 0 ) end
 end
 local Delay_Up = function()
  while EvaluateKeyInt( "$btn" ) == 1 do
   coroutine.yield()
  end
 end
 local proj = GetCurLevel():FindChildEntity("proj")
 local player = GetPlayer()
 local pawn = nil

 while player:GetCurStateDestX() ~= 11 or player:GetCurStateDestY() ~= 9 do coroutine.yield() end
 Delay( 1 )
 GetMasterLevel():BlackOut( 20, 20 )
 Delay( 1 )
 pawn = GetCurLevel():SpawnPreset( "p1" )
 Delay( 50 )
 
 Delay_Down() HeadText( "........", htx_color_0 )
 Delay_Down() HeadText( "Fuck you.", htx_color_0 )
 Delay_Down() HeadText( "Get the fuck off.", htx_color_0 )
 Delay_Down() HeadText( "Don't stand on my way.", htx_color_0 )
 Delay_Down() HeadText( "I want the fuck out.", htx_color_0 )
 Delay_Down() HeadText( "........", htx_color_0 )
 Delay_Down() HeadText( "........", htx_color_x )
 Delay_Down() HeadText( "...", htx_color_sys )
 Delay_Down() HeadText( "......", htx_color_sys )
 Delay_Down() HeadText( "..........", htx_color_sys )

 while true do
  Delay_Down() HeadText( "Error ^31^31^31: Not responding.", htx_color_1 )

  local str1 = "[Y <-- --> N]"
  local str2 = "[N <-- --> Y]"
  local b = RandInt( 0, 2 ) == 0
  Delay_Down() HeadText( "Reboot system?" .. ( b and str1 or str2 ) .. ".", htx_color_1 )
  Delay_Down() if player:GetToX() == ( b and 12 or 10 ) then goto continue end HeadText( "Y", htx_color_1 )
  b = RandInt( 0, 2 ) == 0
  Delay_Down() HeadText( "Are you sure?(1/3)" .. ( b and str1 or str2 ) .. ".", htx_color_1 )
  Delay_Down() if player:GetToX() == ( b and 12 or 10 ) then goto continue end HeadText( "Y", htx_color_1 )
  b = RandInt( 0, 2 ) == 0
  Delay_Down() HeadText( "Are you sure?(2/3)" .. ( b and str1 or str2 ) .. ".", htx_color_1 )
  Delay_Down() if player:GetToX() == ( b and 12 or 10 ) then goto continue end HeadText( "Y", htx_color_1 )
  b = RandInt( 0, 2 ) == 0
  Delay_Down() HeadText( "Are you sure?(3/3)" .. ( b and str1 or str2 ) .. ".", htx_color_1 )
  Delay_Down() if player:GetToX() == ( b and 12 or 10 ) then goto continue end HeadText( "Y", htx_color_1 )
  break
  ::continue::
  HeadText( "N", htx_color_1 )
 end

 Delay_Down() HeadText( "..........", htx_color_sys )
 Delay_Down() HeadText( "System rebooting...", htx_color_sys )
 Delay_Down() HeadText( "..........", htx_color_sys )

 Delay_Down() GetMasterLevel():BlackOut( 5, 5 ) Delay( 1 ) GetCurLevel():RemovePawn( pawn ) SetKeyInt( "$d1", 0 )
 while ( player:GetPosX() == 10 or player:GetPosX() == 12 ) and player:GetPosY() == 8 do coroutine.yield() end
 while not ( ( player:GetToX() == 10 or player:GetToX() == 12 ) and player:GetToY() == 8 ) do coroutine.yield() end
 GetMasterLevel():BlackOut( 30, 100 ) Delay( 80 ) SetKeyInt( "$d1", 1 ) Delay( 15 ) pawn = GetCurLevel():SpawnPreset( "p1" )

 Delay_Down() HeadText( "Initializing system...", htx_color_sys )
 Delay_Down() HeadText( "..........", htx_color_sys )
 Delay_Down() HeadText( "Downloading data...", htx_color_sys )
 Delay_Down() HeadText( "..........", htx_color_sys )
 Delay_Down() GetMasterLevel():BlackOut( 50, 0 )

 Delay_Down() HeadText( "Verifying account...", htx_color_sys )
 Delay_Down() HeadText( "..........", htx_color_sys ) GetMasterLevel():BlackOut( 5, 0 )
 Delay_Down() proj:Follow( player ) GetMasterLevel():BlackOut( 30, 0 )
 Delay_Down() HeadText( "Loading......20%", htx_color_sys ) GetMasterLevel():BlackOut( 5, 0 )
 Delay_Down() HeadText( "..........", htx_color_sys ) GetMasterLevel():BlackOut( 5, 0 )

 Delay_Down() GetMasterLevel():BlackOut( 20, 5 ) Delay( 1 ) GetCurLevel():RemovePawn( pawn ) SetKeyInt( "$d1", 0 )
 Delay( 60 )
 while string.sub( player:GetCurStateName(), 1, 6 ) ~= "attack" do coroutine.yield() end
 GetMasterLevel():BlackOut( 20, 100 ) Delay( 80 ) SetKeyInt( "$d1", 1 ) Delay( 15 ) pawn = GetCurLevel():SpawnPreset( "p1" )

 Delay_Down() HeadText( "Loading......50%", htx_color_sys ) GetMasterLevel():BlackOut( 10, 0 )
 Delay_Down() HeadText( "Loading......60%", htx_color_sys ) GetMasterLevel():BlackOut( 10, 0 )
 GetCurLevel():GetPawnByName( "d1" ):PlayStateForceMove( "", 22 - player:GetToX(), player:GetToY(), 0 )
 Delay_Down() HeadText( "Loading......65%", htx_color_sys ) GetMasterLevel():BlackOut( 10, 0 )
 Delay_Down() HeadText( "Loading......70%", htx_color_sys ) GetMasterLevel():BlackOut( 10, 0 )
 GetCurLevel():GetPawnByName( "d1" ):PlayStateForceMove( "", 4, 4, 0 )
 Delay_Down() HeadText( "Loading......75%", htx_color_sys ) GetMasterLevel():BlackOut( 10, 0 )
 GetCurLevel():GetPawnByName( "d1" ):PlayStateForceMove( "", 7, 7, 0 )
 Delay_Down() HeadText( "Loading......80%", htx_color_sys ) GetMasterLevel():BlackOut( 10, 0 )
 GetCurLevel():GetPawnByName( "d1" ):PlayStateForceMove( "", 11, 9, 0 )
 
 Delay_Down() GetMasterLevel():BlackOut( 20, 5 ) Delay( 1 ) pawn:PlayStateForceMove( "", 4, 4, 0 ) SetKeyInt( "$d1", 0 )
 while player:GetCurStateDestY() >= 6 or player:GetCurStateDestX() >= 8 or GetCurLevel():CheckGrid( player:GetCurStateDestX(), player:GetCurStateDestY() ) < 3 do coroutine.yield() end
 Delay( 1 ) GetMasterLevel():BlackOut( 20, 20 ) pawn:PlayStateForceMove( "", player:GetCurStateDestX(), player:GetCurStateDestY(), 0 )

 Delay_Down() HeadText( "Loading......85%", htx_color_sys ) GetMasterLevel():BlackOut( 15, 0 )
 Delay_Down() GetMasterLevel():BlackOut( 20, 100 ) Delay( 80 ) SetKeyInt( "$d1", 1 ) Delay( 15 ) pawn:PlayStateForceMove( "", 11, 9, 0 )
 Delay_Down() HeadText( "Loading......90%", htx_color_sys ) GetMasterLevel():BlackOut( 15, 0 )
 Delay_Down() HeadText( "..........", htx_color_sys ) GetMasterLevel():BlackOut( 20, 0 )
 Delay_Down() HeadText( "..........", htx_color_2 ) GetMasterLevel():BlackOut( 20, 0 )
 
 Delay_Down() GetMasterLevel():BlackOut( 20, 5 ) Delay( 1 ) pawn:PlayStateForceMove( "", 2, 6, 0 ) SetKeyInt( "$d1", 0 )
 while player:GetCurStateDestX() + player:GetCurStateDestY() > 10 or player:GetCurStateDestY() + player:GetCurStateDestX() < 2 do coroutine.yield() end
 GetMasterLevel():BlackOut( 20, 5 ) Delay( 1 ) pawn:PlayStateForceMove( "stand", player:GetCurStateDestX() + 4, player:GetCurStateDestY(), 1 )
 Delay( 20 ) GetMasterLevel():BlackOut( 30, 15 ) pawn:PlayState( "attack_1", 1 ) Delay( 24 )
 GetMasterLevel():BlackOut( 50, 30 ) Delay( 20 ) pawn:PlayState( "attack_1a_forward", 1 ) Delay( 20 )
 GetMasterLevel():BlackOut( 20, 5 ) GetCurLevel():RemovePawn( pawn )
 Delay( 300 ) GetMasterLevel():BlackOut( 20, 100 ) Delay( 80 ) SetKeyInt( "$d1", 1 ) Delay( 15 )
 pawn = GetCurLevel():SpawnPreset1( "p1", 11, 9, 0 )

 Delay_Down() HeadText( "Loading......94%", htx_color_sys ) GetMasterLevel():BlackOut( 10, 0 ) SetKeyInt( "$d1", 0 )
 for i = 1, 4, 1 do
  Delay_Down() HeadText( "Loading......" .. tostring( i + 94 ) .. "%", htx_color_sys ) GetMasterLevel():BlackOut( 10, 0 )
  local x = player:GetToX()
  local y = player:GetToY()
  local dir = player:GetCurDir()
  local ofs = { { -1, 1 }, { -2, 0 }, { -1, -1 }, { 1, 1 }, { 2, 0 }, { 1, -1 } }
  Delay( 1 ) for k = 1, #ofs, 1 do
   local a = RandInt( k, #ofs + 1 )
   local tmp = ofs[k]
   ofs[k] = ofs[a]
   ofs[a] = tmp
   if pawn:PlayStateForceMove( "", x + ofs[k][1], y + ofs[k][2], dir ) then break end
  end

 end
 Delay_Down() HeadText( "Loading......99%", htx_color_sys )
 GetMasterLevel():BlackOut( 10, 100 ) Delay( 80 ) SetKeyInt( "$d1", 1 ) Delay( 15 ) pawn:PlayStateForceMove( "", 11, 9, 0 )
 Delay_Down() HeadText( "..........", htx_color_1 ) GetMasterLevel():BlackOut( 10, 0 )
 Delay_Down() HeadText( "Loading......99.2%", htx_color_sys ) GetMasterLevel():BlackOut( 10, 0 )
 Delay( 1 ) pawn:PlayStateForceMove( "", player:GetToX() + ( player:GetCurDir() == 0 and -4 or 4 ), player:GetToY(), player:GetCurDir() )
 SetKeyInt( "$d1", 0 )
 while true do
  local x = player:GetCurStateDestX()
  local y = player:GetCurStateDestY()
  if GetCurLevel():CheckGrid( x, y ) == 3 then pawn:PlayStateForceMove( "", x, y, player:GetCurDir() ) break end
  coroutine.yield()
 end
 GetMasterLevel():BlackOut( 50, 10 )
 while player:GetCurStateName() ~= "break" do coroutine.yield() end
 Delay( 10 )
 SetKeyInt( "$d1", 1 )
 Delay( 55 )
 GetMasterLevel():BlackOut( 5, 0 )
 GetCurLevel():RemovePawn( pawn )
 for i = 1, 180, 1 do
  if player:GetCurStateDestX() ~= player:GetPosX() or player:GetCurStateDestY() ~= player:GetPosY() then break end
  coroutine.yield()
 end
 Delay( 4 )
 GetMasterLevel():BlackOut( 10, 0 )
 pawn = GetCurLevel():SpawnPreset( "p1" )

 Delay_Down() HeadText( "Loading......99.5%", htx_color_sys ) GetMasterLevel():BlackOut( 10, 0 )
 Delay_Down() HeadText( "Loading......99.7%", htx_color_sys ) GetMasterLevel():BlackOut( 10, 0 )
 Delay_Down() HeadText( "Loading......99.8%", htx_color_sys ) GetMasterLevel():BlackOut( 10, 0 )
 Delay_Down() HeadText( "Loading......99.9%", htx_color_sys ) GetMasterLevel():BlackOut( 10, 0 )
 Delay_Down() HeadText( "..........", htx_color_sys ) GetMasterLevel():BlackOut( 10, 0 )
 for i = 1, 4, 1 do
  Delay_Down() HeadText( "..........", htx_color_1 ) GetMasterLevel():BlackOut( 10, 0 )
  Delay_Down() HeadText( "..........", htx_color_1 ) GetMasterLevel():BlackOut( 10, 0 )
  
  local x = player:GetToX()
  local y = player:GetToY()
  local dir = player:GetCurDir()
  local ofs = { { -1, 1 }, { -2, 0 }, { -1, -1 }, { 1, 1 }, { 2, 0 }, { 1, -1 } }
  local pawn1 = nil
  Delay( 1 ) for k = 1, #ofs, 1 do
   local a = RandInt( k, #ofs + 1 )
   local tmp = ofs[k]
   ofs[k] = ofs[a]
   ofs[a] = tmp

   if GetCurLevel():CheckGrid( x + ofs[k][1], y + ofs[k][2] ) == 3 and GetCurLevel():CheckGrid( x + ofs[k][1] * 2, y + ofs[k][2] * 2 ) == 3 then
    pawn1 = GetCurLevel():SpawnPreset1( "p1", x + ofs[k][1] * 2, y + ofs[k][2] * 2, ofs[k][1] < 0 and 0 or 1 )
	Delay( 30 )
	GetMasterLevel():BlackOut( 20, 10 ) pawn1:PlayState( "attack_1", 1 ) Delay( 24 )
	local action1 = ofs[k][2] < 0 and "attack_1a_up" or ( ofs[k][2] > 0 and "attack_1a_down" or "attack_1a_forward" )
    GetMasterLevel():BlackOut( 50, 30 ) Delay( 20 ) pawn1:PlayState( action1, 1 ) Delay( 20 )
	GetMasterLevel():BlackOut( 20, 5 ) pawn1:PlayState( "stand", 1 )
	break
   end
  end
  if not pawn1 then break end
 end
 while true do
  Delay_Down() HeadText( "..........", htx_color_1 ) GetMasterLevel():BlackOut( 10, 0 )
  
  local x = player:GetToX()
  local y = player:GetToY()
  local dir = player:GetCurDir()
  local ofs = { { -1, 1 }, { -2, 0 }, { -1, -1 }, { 1, 1 }, { 2, 0 }, { 1, -1 } }
  local pawn1 = nil
  Delay( 1 ) for k = 1, #ofs, 1 do
   local a = RandInt( k, #ofs + 1 )
   local tmp = ofs[k]
   ofs[k] = ofs[a]
   ofs[a] = tmp
   if GetCurLevel():CheckGrid( x + ofs[k][1], y + ofs[k][2] ) == 3 then
    pawn1 = GetCurLevel():SpawnPreset1( "p1", x + ofs[k][1], y + ofs[k][2], ofs[k][1] < 0 and 0 or 1 )
	break
   end
  end
  if not pawn1 then break end
 end
 
 Delay_Down() HeadText( "ERROR: Verification failed.", htx_color_1 )
 Delay_Down() HeadText( "Activating security system.", htx_color_1 )
 Delay_Down() HeadText( "Locking down all areas.", htx_color_1 )
 Delay_Down() HeadText( "Please keep distance.", htx_color_1 )
 
 GetMasterLevel():BlackOut( 50, 20 )
 Delay( 5 )
 local tbl1 = GetCurLevel():GetAllPawnsByNameScript( "p1" )
 for i = 1, #tbl1, 1 do
  GetCurLevel():RemovePawn( tbl1[i] )
 end
 Delay( 5 )
 player:PlayStateForceMove( "break", 5, 5, 0 )
 pawn = nil

 SetKeyInt( "$d1", 0 )
 HeadText( "STAY HERE...FOREVER.", htx_color_h )

 local btn = GetCurLevel():SpawnPreset( "btn" )

 Delay_Down() HeadText( "..........", htx_color_h )
 Delay_Down() HeadText( "It's all over.", htx_color_h )
 Delay_Down() HeadText( "..........", htx_color_h )
 Delay_Down() HeadText( "It's useless.", htx_color_h )
 Delay_Down() HeadText( "..........", htx_color_h )
 Delay_Down() HeadText( "Stop it.", htx_color_h )
 Delay_Down() HeadText( "..........", htx_color_h )
 Delay_Down() HeadText( "You're done.", htx_color_h )
 Delay_Down() HeadText( "..........", htx_color_h )
 Delay_Down() GetMasterLevel():BlackOut( 20, 0 ) for i = 1, 4, 1 do GetCurLevel():SpawnPreset( "sofa" .. tostring( i ) ) end
 Delay_Down() HeadText( "You screwed it up.", htx_color_h )
 Delay_Down() GetMasterLevel():BlackOut( 20, 0 ) for i = 5, 8, 1 do GetCurLevel():SpawnPreset( "sofa" .. tostring( i ) ) end
 Delay_Down() HeadText( "..........", htx_color_h )
 Delay_Down() GetMasterLevel():BlackOut( 20, 0 ) for i = 1, 4, 1 do GetCurLevel():SpawnPreset( "npc" .. tostring( i ) ) end
 Delay_Down() GetMasterLevel():BlackOut( 20, 0 ) for i = 5, 8, 1 do GetCurLevel():SpawnPreset( "npc" .. tostring( i ) ) end
 Delay_Down() HeadText( "You can't fix it.", htx_color_h )
 Delay_Down() local tv = GetCurLevel():SpawnPreset( "tv" ) tv:FindChildEntity( "screen" ):SetVisible( false ) tv:FindChildEntity( "broken" ):SetVisible( false )
 Delay_Down() HeadText( "..........", htx_color_h )
 Delay_Down( true ) tv:FindChildEntity( "screen" ):SetVisible( true )
 for i = 5, 1, -1 do
  while player:GetToY() > i do
   if EvaluateKeyInt( "$btn" ) == 1 then PlaySoundEffect( "door_hit" ) end
   coroutine.yield()
  end
  GetMasterLevel():BlackOut( 40, 20 )
  if i > 1 then PlaySoundEffect( "door_hit" ) end
 end
 Delay( 15 )
 SetKeyInt( "$d2", 1 )
 while player:GetCurStateDestX() ~= 0 or player:GetCurStateDestY() ~= 0 do coroutine.yield() end
 Delay( 1 )
 GetMasterLevel():BlackOut( 20, 20 )
 Delay( 1 )
 pawn = GetCurLevel():SpawnPreset( "p2" )
 Delay( 50 )
 SetKeyInt( "$btn", 0 )

 Delay_Down()
 HeadText( "..........", htx_color_x )
 Delay( 40 ) SetKeyInt( "$btn", 0 ) Delay_Down()
 HeadText( "I will find you.", htx_color_x )
 Delay( 40 ) SetKeyInt( "$btn", 0 ) Delay_Down()
 HeadText( "I will torture you.", htx_color_x )
 GetMasterLevel():BlackOut( 10, 10 ) Delay( 1 ) player:PlayStateForceMove( "stand", 2, 2, 1 ) Delay_Down()
 HeadText( "I will murder you.", htx_color_x )
 GetMasterLevel():BlackOut( 10, 10 ) Delay( 1 ) player:PlayStateForceMove( "stand", 2, 2, 1 ) Delay_Down()

 GetMasterLevel():BlackOut( 20, 20 )
 pawn:PlayState( "break" )
 HeadText( "Get up and speak to me.", htx_color_x )
 Delay( 1 ) player:PlayStateForceMove( "stand", 2, 2, 1 ) Delay_Down()
 GetMasterLevel():BlackOut( 20, 20 )
 pawn:PlayState( "break" )
 HeadText( "Stand up.", htx_color_x )
 Delay( 1 ) player:PlayStateForceMove( "stand", 3, 3, 1 ) Delay_Down()
 GetMasterLevel():BlackOut( 30, 20 )
 pawn:PlayState( "break" )
 HeadText( "STAND UP.", htx_color_x )
 Delay( 1 ) player:PlayStateForceMove( "stand", 4, 4, 1 ) Delay_Down()
 GetMasterLevel():BlackOut( 40, 20 )
 pawn:PlayState( "break" )
 HeadText( "STAAAAAAND UUUUUUUUPPPPPPPPP.", htx_color_x )
 Delay( 1 ) player:PlayStateForceMove( "stand", 5, 5, 1 ) Delay_Down()

 Delay( 10 )
 GetMasterLevel():BlackOut( 50, 20 )
 Delay( 10 )
 player:PlayState( "break" )
 HeadText( "TOO CLOSE.", htx_color_2, 240 )
 local src = proj:GetProjSrc()
 local dst = { ( player:GetPosX() + 1 ) * LEVEL_GRID_SIZE_X, player:GetPosY() * LEVEL_GRID_SIZE_Y + 16 }
 local l = CreateLighningEft( src, dst )
 GetMasterLevel():InterferenceStripEffect( 1, 1 )
 Delay( 60 )
 GetMasterLevel():BlackOut( 20, 10 )
 Delay( 5 )
 player:PlayStateForceMove( "break", 5, 5, 1 )
 Delay( 65 )
 GetMasterLevel():InterferenceStripEffect( 0, 0 )
 l:SetParentEntity( nil )
 
 GetMasterLevel():BlackOut( 50, 20 )
 GetCurLevel():RemovePawn( pawn )
 local tblSmoke = { { { 0, 0 }, { 2, 0 } }, { { 1, 1 }, { 3, 1 } }, { { 2, 2 }, { 4, 2 } }, { { 3, 3 }, { 5, 3 }, { 7, 3 } },
 { { 4, 4 }, { 6, 4 }, { 8, 4 }, { 10, 4 } }, { { 3, 5 }, { 5, 5 }, { 7, 5 }, { 9, 5 }, { 11, 5 }, { 13, 5 } },
 { { 2, 6 }, { 4, 6 }, { 6, 6 }, { 8, 6 }, { 10, 6 }, { 12, 6 }, { 14, 6 }, { 16, 6 } } }
 local Flood = function( n )
  local tbl = tblSmoke[n]
  for i = 1, #tbl, 1 do
   local pos = tbl[i]
   tbl[i] = GetCurLevel():SpawnPawn( 0, pos[1], pos[2], 0 )
  end
  PlaySoundEffect( "spray" )
 end
 Flood( 1 )

 Delay_Down( true )
 Delay_Up()
 GetMasterLevel():BlackOut( 20, 0 )
 GetCurLevel():RemovePawn( btn )
 btn = GetCurLevel():SpawnPreset( "sofa0" )

 local Sofa_Begin = function() Delay_Down( true ) GetCurLevel():BeginNoise( "noise_tv" ) end
 local Sofa_End = function() Delay_Up() GetCurLevel():EndNoise( "noise_tv" ) end
 
 Sofa_Begin() HeadText( "..........", htx_color_h ) Sofa_End() GetMasterLevel():BlackOut( 10, 0 ) Flood( 2 )
 Sofa_Begin() HeadText( "This is a tragedy.", htx_color_h ) Sofa_End() Sofa_End() GetMasterLevel():BlackOut( 10, 0 ) Flood( 3 )
 Sofa_Begin() HeadText( "..........", htx_color_h ) Sofa_End() Sofa_End() GetMasterLevel():BlackOut( 10, 0 ) Flood( 4 )
 Sofa_Begin() HeadText( "This is a catastrophe.", htx_color_h ) Sofa_End() Sofa_End() GetMasterLevel():BlackOut( 10, 0 ) Flood( 5 )
 Sofa_Begin() HeadText( "..........", htx_color_h ) Sofa_End() Sofa_End() GetMasterLevel():BlackOut( 10, 0 ) Flood( 6 )
 Sofa_Begin() HeadText( "This is our fault.", htx_color_h ) Sofa_End() GetMasterLevel():BlackOut( 10, 0 ) Flood( 7 )
 Sofa_Begin() HeadText( "We are all to blame for this.", htx_color_h ) Sofa_End()
 GetMasterLevel():BlackOut( 30, 10 ) tv:FindChildEntity( "broken" ):SetVisible( true ) PlaySoundEffect( "glass_break" ) Delay( 5 )
 while true do if not GetCurLevel():SpawnPreset( "g" ) then break end end

 Sofa_Begin() HeadText( "We must fix this.", htx_color_h ) Sofa_End()
 GetMasterLevel():BlackOut( 30, 10 ) Delay( 5 )
 for i = 1, 8, 1 do
  GetCurLevel():RemovePawn( GetCurLevel():GetPawnByName( "npc" .. tostring( i ) ) )
 end
 while true do if not GetCurLevel():SpawnPreset( "s" ) then break end end

 Sofa_Begin() HeadText( "We must reconstruct this world.", htx_color_h ) Sofa_End()
 GetMasterLevel():BlackOut( 30, 10 ) Delay( 5 )
 for i = 1, 8, 1 do
  GetCurLevel():RemovePawn( GetCurLevel():GetPawnByName( "sofa" .. tostring( i ) ) )
 end
 while true do if not GetCurLevel():SpawnPreset( "t" ) then break end end

 Sofa_Begin() HeadText( "Connect us all together.", htx_color_h ) Sofa_End()
 Sofa_Begin() HeadText( "Purge the shit in our head.", htx_color_h ) Sofa_End()
 Sofa_Begin() HeadText( "Give us a new mind.", htx_color_h ) Sofa_End()
 GetMasterLevel():BlackOut( 30, 10 ) Delay( 5 ) GetCurLevel():RemovePawn( btn ) tv:FindChildEntity( "screen" ):SetVisible( false )
 GetCurLevel():SpawnPreset( "fall" )
 SetKeyInt( "d4_t4_unknown_finished", 1 )
end


function Day4_4_3f_btn()
 if not( CurDay() == 4 and CurTime() == 4 and EvaluateKeyInt( "day4_3f_box_state" ) > 0 and FEVT( "$sc1" ) ) then return end
 HeadText( "...!??", htx_color_0, 240 )
 SetKeyInt( "$door0", 1 )

 local Delay_Down = function( b )
  while EvaluateKeyInt( "$btn" ) == 0 do
   coroutine.yield()
  end
  if not b then SetKeyInt( "$btn", 0 ) end
 end
 local Delay_Up = function()
  while EvaluateKeyInt( "$btn" ) == 1 do
   coroutine.yield()
  end
 end
 local proj = GetCurLevel():FindChildEntity("proj")
 local player = GetPlayer()
 local pawn = nil
 
 Delay_Down() HeadText( "^31^31^31^31^31 ^31^31^31^31^31^31^31^31^31^31^31^31^31", htx_color_h, 0, true )
 Delay_Down() HeadText( "Stop moving.", htx_color_h, 0, true ) Delay( 20 ) PlaySoundEffect( "electric1" )
 Delay_Down() HeadText( "^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31", htx_color_h, 0, true )
 Delay_Down() HeadText( "..........", htx_color_h, 0, true ) Delay( 20 ) PlaySoundEffect( "electric1" )
 Delay_Down() HeadText( "Where are you going?", htx_color_h, 0, true ) Delay( 20 ) PlaySoundEffect( "electric1" )
 Delay_Down() HeadText( "^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31", htx_color_h, 0, true )
 Delay_Down() HeadText( "..........", htx_color_h, 0, true ) Delay( 20 ) PlaySoundEffect( "electric1" )
 Delay_Down() HeadText( "Where do you think you can escape?", htx_color_h, 0, true )
 Delay_Down() HeadText( "..........", htx_color_h, 0, true ) Delay( 20 ) PlaySoundEffect( "electric1" )
 Delay_Down() HeadText( "Stop doing this. It's useless.", htx_color_h, 0, true ) Delay( 20 ) PlaySoundEffect( "electric1" )
 Delay_Down() HeadText( "You know there is no way o^31^31^31^31^31^31^31.", htx_color_h, 0, true )
 Delay_Down() HeadText( "..........", htx_color_h, 0, true ) Delay( 20 ) PlaySoundEffect( "electric1" )
 Delay_Down() HeadText( "I control this p^31^31^31^31^31^31^31^31", htx_color_h, 0, true )
 Delay_Down() HeadText( "^31^31^31^31^31^31 ^31^31^31^31 ^31^31^31^31^31^31^31^31^31^31^31", htx_color_h, 0, true )
 Delay_Down() HeadText( "You know you can't^31^31^31^31^31^31.", htx_color_h, 0, true )
 Delay_Down() HeadText( "..........", htx_color_h, 0, true )
 Delay_Down() HeadText( "No shi^31^31 ^31^31^31^31^31 ^31^31^31^31^31^31", htx_color_h, 0, true )
 Delay_Down() HeadText( "..........", htx_color_h, 0, true )
 Delay_Down() HeadText( "Don't make me^31  ^31   ^31^31^31^31 ^31  ^31^31^31^31^31^31 ^31 ^31^31^31", htx_color_h, 0, true )
 Delay_Down() HeadText( "..........", htx_color_h, 0, true )
 Delay_Down() HeadText( "^31 ^31^31^31^31^31^31^31 ^31^31^31^31 ^31   ^31^31    ^31  ^31   ^31 ^31^31^31^31^31^31^31 ^31^31^31 ^31^31^31 ^31^31^31^31^31^31^31", htx_color_h, 0, true )
 Delay_Down() HeadText( "......", htx_color_h, 0, true )
 Delay_Down() HeadText( "..........", htx_color_h, 0, true )
 Delay_Down() HeadText( "....................", htx_color_h, 0, true )
 Delay_Down() HeadText( "..............................................................................................................................................................................................", htx_color_h, 0, true )
 Delay_Down() PlaySoundEffect( "alert" )
 
 Delay_Down() GetMasterLevel():BlackOut( 60, 45 ) Delay( 30 ) SetKeyInt( "$door", 1 )
 while player:GetCurStateDestX() ~= 7 or player:GetCurStateDestY() ~= 5 do coroutine.yield() end
 GetCurLevel():SpawnPreset( "shadow" ) Delay( 10 )
 PlaySoundEffect( "release" )
 GetMasterLevel():BlackOut( 90, 10 )
 Delay( 5 ) for i = 1, 6, 1 do GetCurLevel():SpawnPreset( "s" .. tostring( i ) ) end
end


function Day4_4_3f_dining()
 local tbl0 = GetCurLevel():GetAllPawnsByNameScript( "0" )
 local player = GetPlayer()
 while player:GetCurStateDestX() == player:GetPosX() and player:GetCurStateDestY() == player:GetPosY() do coroutine.yield() end
 Delay( 30 )
 HeadText( "*cough*", htx_color_0, 240 )
 Delay( 150 )
 GetMasterLevel():BlackOut( 10, 0 )
 player:PlayState( "special_1" )
 PlaySoundEffect( "bzzz0" )
 GetMasterLevel():InterferenceStripEffect( 1, 30 )
 Delay( 60 )
 GetMasterLevel():BlackOut( 10, 0 )
 player:PlayState( "stand" )
 GetMasterLevel():InterferenceStripEffect( 0, 0 )

 while EvaluateKeyInt( "$n" ) == 0 do coroutine.yield() end
 while player:GetCurStateDestX() == player:GetPosX() and player:GetCurStateDestY() == player:GetPosY() do coroutine.yield() end
 Delay( 120 )
 HeadText( "*coughcough*", htx_color_0, 240 )
 Delay( 30 )
 GetMasterLevel():BlackOut( 10, 0 )
 player:PlayState( "special_1" )
 PlaySoundEffect( "bzzz0" )
 GetMasterLevel():InterferenceStripEffect( 1, 30 )
 Delay( 60 )
 GetMasterLevel():BlackOut( 30, 0 )
 player:PlayState( "stand" )
 GetMasterLevel():InterferenceStripEffect( 0, 0 )
 local pawn = GetCurLevel():SpawnPreset( "a" )

 local n0 = 1
 local F1 = function( n )
  for i = 1, n, 1 do
   coroutine.yield( 1 )
   if EvaluateKeyInt( "$n" ) > n0 then GetCurLevel():RemovePawn( pawn ) pawn = nil GetMasterLevel():BlackOut( 45, 0 ) return false end
  end
  return true
 end

 while true do
  for i = 1, 3, 1 do
   if not F1( 240 ) then goto break1 end
   PlaySoundEffect( "bzzz0" )
   GetMasterLevel():BlackOut( 20, 0 )
  end
  for i = 1, 4, 1 do
   if not F1( 120 ) then goto break1 end
   PlaySoundEffect( "bzzz0" )
   GetMasterLevel():BlackOut( 10, 0 )
  end
  for i = 1, 6, 1 do
   if not F1( 60 ) then goto break1 end
   PlaySoundEffect( "bzzz0" )
   GetMasterLevel():BlackOut( 5, 0 )
  end
  for i = 1, 12, 1 do
   if not F1( 15 ) then goto break1 end
   PlaySoundEffect( "bzzz0" )
   GetMasterLevel():BlackOut( 3, 0 )
  end
  GetCurLevel():RemovePawn( pawn )
  pawn = GetCurLevel():SpawnPreset( "a" )
 end
 ::break1::
 
 while player:GetCurStateDestX() == player:GetPosX() and player:GetCurStateDestY() == player:GetPosY() do coroutine.yield() end
 Delay( 150 )
 GetMasterLevel():BlackOut( 20, 0 )
 player:PlayState( "special_1" )
 GetMasterLevel():InterferenceStripEffect( 1, 30 )
 Delay( 120 )
 player:PlayState( "stand" )
 GetMasterLevel():InterferenceStripEffect( 0, 0 )
 GetMasterLevel():BlackOut( 60, 0 )
 pawn = GetCurLevel():SpawnPreset( "b" )

 n0 = 2
 while true do
  for i = 1, 3, 1 do
   if not F1( 240 ) then goto break2 end
   PlaySoundEffect( "bzzz0" )
   GetMasterLevel():BlackOut( 20, 0 )
  end
  for i = 1, 4, 1 do
   if not F1( 120 ) then goto break2 end
   PlaySoundEffect( "bzzz0" )
   GetMasterLevel():BlackOut( 10, 0 )
  end
  for i = 1, 6, 1 do
   if not F1( 60 ) then goto break2 end
   PlaySoundEffect( "bzzz0" )
   GetMasterLevel():BlackOut( 5, 0 )
  end
  for i = 1, 12, 1 do
   if not F1( 15 ) then goto break2 end
   PlaySoundEffect( "bzzz0" )
   GetMasterLevel():BlackOut( 3, 0 )
  end
  GetCurLevel():RemovePawn( pawn )
  pawn = GetCurLevel():SpawnPreset( "b" )
 end
 ::break2::
 
 while player:GetToX() == player:GetPosX() and player:GetToY() == player:GetPosY() do coroutine.yield() end
 GetMasterLevel():BlackOut( 30, 10 )
 Delay( 5 )
 GetCurLevel():RemovePawn( GetCurLevel():GetPawnByName( "cupboard" ) )
 local cupboard1 = GetCurLevel():SpawnPreset( "cupboard1" )
 Delay( 80 )
 for i = 1, 3, 1 do
  Delay( 60 )
  HeadText( "*cccccoughhhhh*", htx_color_0, 240 )
  Delay( 30 )
  GetMasterLevel():BlackOut( 20, 0 )
  player:PlayState( "special_1" )
  GetMasterLevel():InterferenceStripEffect( 1, 30 )
  Delay( 60 )
  player:PlayState( "stand" )
  GetMasterLevel():InterferenceStripEffect( 0, 0 )
  GetMasterLevel():ShowInteractionUI( nil, "data/interaction/scenario1.pf" )
  while player:GetCurStateDestX() == player:GetPosX() and player:GetCurStateDestY() == player:GetPosY() do coroutine.yield() end
 end
 GetMasterLevel():BlackOut( 60, 0 )
 pawn = GetCurLevel():SpawnPreset( "c" )
 
 n0 = 3
 SetKeyInt( "$n0", 1 )

 for i = 1, 2, 1 do
  SetKeyInt( "$n", 3 )
  while true do
   if not F1( 240 ) then break end
   GetMasterLevel():BlackOut( 5, 0 )
   player:PlayState( "special_1" )
   GetMasterLevel():InterferenceStripEffect( 1, 30 )
   Delay( 30 )
   GetMasterLevel():BlackOut( 5, 0 )
   player:PlayState( "stand" )
   GetMasterLevel():InterferenceStripEffect( 0, 0 )
  end
  if pawn then GetCurLevel():RemovePawn( pawn ) end
  
  GetMasterLevel():ShowInteractionUI( nil, "data/interaction/scenario1.pf" )
  Delay( 90 )
  
  while true do
   GetMasterLevel():BlackOut( 5, 0 )
   player:PlayState( "special_1" )
   GetMasterLevel():InterferenceStripEffect( 1, 30 )
   Delay( 30 )
   GetMasterLevel():BlackOut( 5, 0 )
   player:PlayState( "stand" )
   GetMasterLevel():InterferenceStripEffect( 0, 0 )
   local x = player:GetPosX()
   local y = player:GetPosY()
   local l1 = 0
   while GetCurLevel():CheckGrid( x - 2 * ( l1 + 1 ), y ) >= 3 do l1 = l1 + 1 end
   local l2 = 0
   while GetCurLevel():CheckGrid( x + 2 * ( l2 + 1 ), y ) >= 3 do l2 = l2 + 1 end
   local nDir
   if math.max( l1, l2 ) > 2 then
    if l1 + RandInt( 0, 2 ) > l2 then x = x - l1 * 2 nDir = 0
    else x = x + l2 * 2 nDir = 1 end
    pawn = GetCurLevel():SpawnPreset1( "c", x, y, nDir )
	Delay( 1 )
    GetMasterLevel():BlackOut( 40, 10 )
    break
   end
   Delay( 90 )
  end
 end
 
 SetKeyInt( "$n1", 1 )
 SetKeyInt( "$n0", 2 )
 local tblText = { "Stop resisting...", "Not enough...", "More..." }
 for i = 1, 3, 1 do
  HeadText( tblText[i], htx_color_h )
  SetKeyInt( "$n", 3 )
  while true do
   coroutine.yield( 1 )
   if EvaluateKeyInt( "$n" ) > 3 then
    if i == 1 then
     SetKeyInt( "$n", 3 )
     GetMasterLevel():ShowInteractionUI( nil, "data/interaction/scenario1.pf" )
	 Delay( 1 )
     if EvaluateKeyInt( "$n1" ) > 8 then GetCurLevel():RemovePawn( pawn ) pawn = nil GetMasterLevel():BlackOut( 45, 0 ) break end
	else
     SetKeyInt( "$n0", 2 + i )
	 GetMasterLevel():ShowInteractionUI( nil, "data/interaction/scenario1.pf" )
	 Delay( 1 )
	 GetCurLevel():RemovePawn( pawn ) pawn = nil GetMasterLevel():BlackOut( 45, 0 ) break
	end
   end
  end
  
  local tbl = GetCurLevel():GetAllPawnsByNameScript( tostring( 4 - i ) )
  for i1 = 1, #tbl, 1 do
   GetCurLevel():RemovePawn( tbl[i1] )
  end
  if i < 3 then
   pawn = GetCurLevel():SpawnPreset( "c" .. tostring( i ) )
  end
 end

 cupboard1:PlayState( "stand_ready" )

end

function Day4_4_2f_final()
 HeadText( "What are you doing here?.", htx_color_h, 0, true )
 local player = GetPlayer()
 while player:GetPosX() ~= 7 or player:GetPosY() ~= 9 do coroutine.yield() end
 
 player:PlayState( "special_1" )
 GetMasterLevel():InterferenceStripEffect( 1, 100 )
 Delay( 40 )
 player:PlayState( "stand" )
 GetMasterLevel():InterferenceStripEffect( 0, 0 )
 SetKeyInt( "$n0", 6 )
 GetMasterLevel():ShowInteractionUI( nil, "data/interaction/scenario1.pf" )
 player:SetHp( player:GetMaxHp() )

 GetMasterLevel():BlackOut( 30, 10 )
 PlaySoundEffect( "bzzz0" )
 Delay( 1 )
 local tbl = GetCurLevel():GetAllPawnsByNameScript( "1" )
 for i1 = 1, #tbl, 1 do
  GetCurLevel():RemovePawn( tbl[i1] )
 end
 local tblBox = {}
 for i = 1, 6, 1 do
  tblBox[i] = GetCurLevel():SpawnPreset( "b" .. tostring( i ) )
 end
 local tbl1 = {}
 for i = 1, 6, 1 do
  while true do
   coroutine.yield( 1 )
   local bBreak = false
   if not player:GetCurMountingPawn() then for i1 = 1, 6, 1 do local b = tblBox[i1] if b then
    local x = b:GetPosX()
    local y = b:GetPosY()
    local dx = math.abs( x - 7 )
    local dy = math.abs( y - 5 )
    local d = dy + math.max( 0, dx - dy ) / 2
    if d > 2 then
     GetMasterLevel():BlackOut( 30, 0 )
     GetCurLevel():RemovePawn( b )
     tbl1[i] = GetCurLevel():SpawnPreset1( "a" .. tostring( i ), x, y, 0 )
     player:SetHp( player:GetMaxHp() )
     tblBox[i1] = nil
	 bBreak = true
	 break
    end
   end end end
   if bBreak then break end
  end
  if i == 2 then
   HeadText( "Go back. Stop making trouble everywhere.", htx_color_h, 0, true )
  elseif i == 3 then
   HeadText( "Stop this. The final warning.", htx_color_h, 0, true )
  elseif i == 4 then
   HeadText( "I said stop this. Are you fucking listening?", htx_color_h, 0, true )
  elseif i == 5 then
   HeadText( "Are you still fucking sane?", htx_color_h, 0, true )
  end
 end
 
 GetMasterLevel():BlackOut( 30, 0 )
 local proj = GetCurLevel():FindChildEntity("proj")
 proj:Follow( player )
 local src = proj:GetProjSrc()
 local dst = { ( player:GetPosX() + 1 ) * LEVEL_GRID_SIZE_X, ( player:GetPosY() + 1 ) * LEVEL_GRID_SIZE_Y }
 local l = CreateLighningEft( src, dst )
 GetMasterLevel():InterferenceStripEffect( 1, 2 )
 Delay( 60 )
 GetMasterLevel():BlackOut( 20, 10 )
 GetMasterLevel():InterferenceStripEffect( 0, 0 )
 l:SetParentEntity( nil )

 Delay( 240 )
 
 GetMasterLevel():BlackOut( 30, 20 )
 for i = 1, 6, 1 do
  local x = tbl1[i]:GetPosX()
  local y = tbl1[i]:GetPosY()
  GetCurLevel():RemovePawn( tbl1[i] )
  tbl1[i] = GetCurLevel():SpawnPreset1( "bot", x, y, 0 )
 end

 HeadText( "Get the fuck sobered up. Look at what you've done.", htx_color_h, 0, true )
 dst = { ( player:GetPosX() + 1 ) * LEVEL_GRID_SIZE_X, ( player:GetPosY() + 1 ) * LEVEL_GRID_SIZE_Y }
 l = CreateLighningEft( src, dst )
 GetMasterLevel():InterferenceStripEffect( 1, 100 )
 player:PlayState( "special_1" )
 Delay( 240 )
 player:PlayState( "break" )
 Delay( 60 )
 GetMasterLevel():BlackOut( 20, 5 )
 GetMasterLevel():InterferenceStripEffect( 0, 0 )
 l:SetParentEntity( nil )
 
 HeadText( "How to get the fuck out...", htx_color_0, 0, true )
 while player:GetToX() ~= 6 or player:GetToY() ~= 10 do coroutine.yield( 1 ) end
 HeadText( "WAKE UP.", htx_color_h, 0, true )
 dst = { ( player:GetPosX() + 1 ) * LEVEL_GRID_SIZE_X, ( player:GetPosY() + 1 ) * LEVEL_GRID_SIZE_Y }
 l = CreateLighningEft( src, dst )
 GetMasterLevel():InterferenceStripEffect( 1, 100 )
 player:PlayState( "break" )
 Delay( 50 )
 GetMasterLevel():BlackOut( 30, 30 )
 Delay( 5 )
 local tblPos = { { 9, 5 }, { 5, 5 }, { 8, 6 }, { 6, 6 }, { 8, 4 }, { 6, 4 } }
 for i = 1, 6, 1 do
  GetCurLevel():RemovePawn( tbl1[i] )
  tbl1[i] = GetCurLevel():SpawnPreset1( "bot", tblPos[i][1], tblPos[i][2], 0 )
 end
 Delay( 5 )
 player:PlayStateForceMove( "", 7, 5, player:GetCurDir() )
 for i = 1, 6, 1 do
  tbl1[i]:PlayState( "stand_pause" )
 end
 GetMasterLevel():InterferenceStripEffect( 0, 0 )
 l:SetParentEntity( nil )
 
 HeadText( "ERROR: Evacuating sewer system severely damaged.", htx_color_1, 0, true )
 Delay( 300 )
 HeadText( "What's fucking wrong with you?", htx_color_h, 0, true )
 Delay( 240 )
 HeadText( "Did you take in some..that shit?", htx_color_h, 0, true )
 Delay( 240 )
 HeadText( "I'm getting regret having brought you in.", htx_color_h, 0, true )
 Delay( 240 )
 
 dst = { ( player:GetPosX() + 1 ) * LEVEL_GRID_SIZE_X, ( player:GetPosY() + 1 ) * LEVEL_GRID_SIZE_Y }
 l = CreateLighningEft( src, dst )
 GetMasterLevel():InterferenceStripEffect( 1, 100 )
 player:PlayState( "break" )
 Delay( 50 )
 GetMasterLevel():BlackOut( 120, 0 )
 ClearKeys()
 player:Reset( 0 )
 SetKeyInt( "day", 4 )
 SetCurTime( 5 )
 TransferTo( "stages/4f_0.pf", 8, 2, 0, -1 )
end