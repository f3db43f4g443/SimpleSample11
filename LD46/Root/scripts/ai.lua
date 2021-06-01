function AIFunc_GarbageBin_Damage( pawn, damage, type, x, y )
 if type > 0 and pawn:GetCurStateIndex() == 0 then
  local nDamageDir = pawn:GetDamageOfsDir1( x, y )
  if nDamageDir == 0 or nDamageDir == 1 then
   pawn:SetDamaged( type, x, y )
   return true, damage
  end
 end
 return true, 0
end

function AIFunc_GarbageBin( pawn, cur, dir )
 if cur == 0 and pawn:IsDamaged() then
  local nDamageDir = pawn:GetDamageOfsDir()
  if nDamageDir == 0 or nDamageDir == 1 then
   return 2, nDamageDir
  end
 end
 return -1, dir
end

function AIFunc_GarbageBin_1_Damage( pawn, damage, type, x, y )
 if type > 0 and pawn:GetCurStateIndex() == 0 then
  local nDamageDir = pawn:GetDamageOfsDir1( x, y )
  local dir = pawn:GetCurDir()
  if nDamageDir == 4 + dir or nDamageDir == 2 + 1 - dir then
   pawn:SetDamaged( type, x, y )
   return true, damage
  end
 end
 return true, 0
end

function AIFunc_GarbageBin_1( pawn, cur, dir )
 if cur == 0 and pawn:IsDamaged() then
  local nDamageDir = pawn:GetDamageOfsDir()
  if nDamageDir == 4 + dir then
   return 2, dir
  elseif nDamageDir == 2 + 1 - dir then
   return 4, dir
  end
 end
 return -1, dir
end


function AIFunc_Lever_Damage( pawn, damage, type, x, y )
 if pawn:IsLocked() then return true, 0 end
 local nCurState = pawn:GetCurStateIndex()
 if x > 0 and nCurState == 1 or x < 0 and nCurState == 2 then
  pawn:SetDamaged( type, x, y )
  return true, damage
 end
 return true, 0
end

function AIFunc_Lever( pawn, cur, dir )
 if pawn:IsDamaged() then
  local x = pawn:GetDamageOfsX()
  if x > 0 and cur == 1 or x < 0 and cur == 2 then
   return cur + 2, dir
  end
 end
 return -1, dir
end

function AIFunc_Lever_Signal( pawn, n )
 if n == 0 then
  PlayerPickUp( "data/pickups/staff.pf" )
 end
 return 0
end

function AIFunc_Power_Well_Signal( pawn, n )
 if n == 0 then
  PlayerPickUp( "data/pickups/staff.pf" )
  local equip = GetPlayer():GetEquipment( 3 )
  equip:SetAmmo( pawn:GetAI():GetIntValue( "charge" ) )
 elseif n == 1 then
  local equip = GetPlayer():GetEquipment( 3 )
  local curCharge = pawn:GetAI():GetIntValue( "charge" )
  if equip then
   local nCharge = math.max( curCharge, equip:GetAmmo() )
   pawn:GetAI():SetIntValue( "charge", nCharge )
   equip:SetAmmo( nCharge )
  end
 elseif n == 2 then
  GetPlayer():RemoveEquipment( 3 )
 end
 return 0
end

function AIFunc_Bot_Damage( pawn, damage, type, x, y )
 if pawn:GetCurStateName() == "stand_0" then
  pawn:SetDamaged( type, x, y )
  Signal( pawn, 0 )
  return true, damage
 end
 return true, 0
end

function AIFunc_Bot_Delay( n, pawn )
 for i = 2, n - 1, 1 do
  coroutine.yield()
  if pawn:GetCurStateName() ~= "stand_0" then return true end
 end
 return false
end


function AIFunc_PlayerTracer_PreInit( pawn )
 for i = 0, 3, 1 do
  local strPickUp = EvaluateKeyString( "player_tracer_eq" .. tostring( i ) )
  if #strPickUp > 0 then
   PlayerPickUp( strPickUp, pawn )
  end
 end
 pawn:RestoreAmmo( -1 )
 if GetLabelKey( "_FOOD" ) == 0 then
  pawn:FindChildEntity( "mount_0" ):SetEnabled( false )
  pawn:FindChildEntity( "mount_1" ):SetEnabled( false )
 end
 local strHeadText = EvaluateKeyString( "player_tracer_lost_weapon_ht" )
 if #strHeadText > 0 then
  HeadText( strHeadText, htx_color_0, 240 )
  SetKeyString( "player_tracer_lost_weapon_ht", "" )
 end
 return EvaluateKeyInt( "player_tracer_sc" ) > 0 and 0 or 11
end

function AIFunc_PlayerTracer_LevelEnd( ai, pawn )
 for i = 0, 3, 1 do
  if ai:IsOrigWeaponLost( i ) and ai:GetCurEquipName( i ) ~= "knife" then
   SetKeyString( "player_tracer_eq" .. tostring( i ), "" )
   if FEVT( "player_tracer_lost_weapon_0" ) then
    SetKeyString( "player_tracer_lost_weapon_ht", "Put it down motherfucker." )
   end
  end
 end
end

function AIFunc_PlayerTracer_Action( ai, pawn )
 local szCurState = ""
 local nCurDir = 0
 local nxtX = 0
 local nxtY = 0
 local target = nil

 local function Wait( szState, nDelay, szNewState, nDir, nType )
  if szState then
   while szCurState ~= szState do
    szCurState, nCurDir = coroutine.yield()
   end
  end
  if nDelay then
   for i = 1, nDelay, 1 do
    szCurState, nCurDir = coroutine.yield()
   end
  end
  if szNewState or nType then
   szCurState, nCurDir = coroutine.yield( szNewState, nDir, nType )
  end
 end
 local function DoAction( szNewState, nDir, nType )
   szCurState, nCurDir = coroutine.yield( szNewState, nDir, nType )
 end

 local function Func1( n )
  local x0 = pawn:GetToX()
  local y0 = pawn:GetToY()
  local x1 = target:GetToX()
  local y1 = target:GetToY()
  local dx1 = x1 - x0
  local dy1 = y1 - y0
  local x2 = target:GetPosX()
  local y2 = target:GetPosY()
  local dx2 = x2 - x0
  local dy2 = y2 - y0

  if nCurDir > 0 then
   dx1 = -dx1
   dx2 = -dx2
  end
  if n == 1 then
   if dx1 == 2 and dy1 == 0 or dx2 == 2 and dy2 == 0 then DoAction( "attack_1", nCurDir ) Func1( 2 ) return true end
   if dx1 == 1 and dy1 == 1 or dx2 == 1 and dy2 == 1 then DoAction( "attack_1", nCurDir ) Func1( 2 ) return true end
   if dx1 == 1 and dy1 == -1 or dx2 == 1 and dy2 == -1 then DoAction( "attack_1", nCurDir ) Func1( 2 ) return true end
  end

  if n <= 2 then
   local b1 = PawnCanMoveTo( pawn, pawn:GetToX() + ( nCurDir > 0 and -2 or 2 ), pawn:GetToY() )
   local b2 = PawnCanMoveTo( pawn, pawn:GetToX() + ( nCurDir > 0 and -1 or 1 ), pawn:GetToY() + 1 )
   local b3 = PawnCanMoveTo( pawn, pawn:GetToX() + ( nCurDir > 0 and -1 or 1 ), pawn:GetToY() - 1 )
   if b1 and ( dx1 == 4 and dy1 == 0 or dx2 == 4 and dy2 == 0 ) then DoAction( "attack_1a_forward", nCurDir ) Func1( 3 ) return true end
   if b2 and ( dx1 == 2 and dy1 == 2 or dx2 == 2 and dy2 == 2  ) then DoAction( "attack_1a_up", nCurDir ) Func1( 3 ) return true end
   if b3 and ( dx1 == 2 and dy1 == -2 or dx2 == 2 and dy2 == -2  ) then DoAction( "attack_1a_down", nCurDir ) Func1( 3 ) return true end
   if b2 and ( dx1 == 0 and dy1 == 2 or dx2 == 0 and dy2 == 2 ) then DoAction( "attack_1a_up", nCurDir ) Func1( 3 ) return true end
   if b3 and ( dx1 == 0 and dy1 == -2 or dx2 == 0 and dy2 == -2 ) then DoAction( "attack_1a_down", nCurDir ) Func1( 3 ) return true end
   if ( dx1 == 3 and dy1 == 1 or dx2 == 3 and dy2 == 1 ) and ( b1 or b2 ) then
    if not b1 or b2 and RandInt( 0, 2 ) == 1 then
	 DoAction( "attack_1a_up", nCurDir ) Func1( 3 ) return true
	else
     DoAction( "attack_1a_forward", nCurDir ) Func1( 3 ) return true
	end
   end
   if ( dx1 == 3 and dy1 == -1 or dx2 == 3 and dy2 == 1 ) and ( b1 or b3 ) then
    if not b1 or b3 and RandInt( 0, 2 ) == 1 then
	 DoAction( "attack_1a_down", nCurDir ) Func1( 3 ) return true
	else
     DoAction( "attack_1a_forward", nCurDir ) Func1( 3 ) return true
	end
   end
  end
  
  if dx1 == 2 and dy1 == 0 or dx2 == 2 and dy2 == 0 then DoAction( "attack_1b", n == 1 and 1 - nCurDir or nCurDir ) return true end
  if dx1 == -2 and dy1 == 0 or dx2 == -2 and dy2 == 0 then DoAction( "attack_1b", n == 1 and 1 - nCurDir or nCurDir ) return true end
  if dx1 == 1 and dy1 == 1 or dx2 == 1 and dy2 == 1 then DoAction( "attack_1b", n == 1 and 1 - nCurDir or nCurDir ) return true end
  if dx1 == -1 and dy1 == 1 or dx2 == -1 and dy2 == 1 then DoAction( "attack_1b", n == 1 and 1 - nCurDir or nCurDir ) return true end
  if dx1 == 1 and dy1 == -1 or dx2 == 1 and dy2 == -1 then DoAction( "attack_1b", n == 1 and 1 - nCurDir or nCurDir ) return true end
  if dx1 == -1 and dy1 == -1 or dx2 == -1 and dy2 == -1 then DoAction( "attack_1b", n == 1 and 1 - nCurDir or nCurDir ) return true end
  return false
 end

 while true do
  Wait( "stand", 0, nil, nil, 1 )
  nxtX, nxtY, target = ai:FindTarget( target )
  local bAction = false
  if target then
   bAction = Func1( 1 )
  end

  local nWait = 30
  if not bAction then
   if nxtX >= 0 then
    local x0 = pawn:GetToX()
    local y0 = pawn:GetToY()
    local dx = nxtX - x0
    local dy = nxtY - y0

    local dx1 = math.abs( target:GetToX() - nxtX )
    local dy1 = math.abs( target:GetToY() - nxtY )
    if dx1 == 2 and dy1 == 0 or dx1 == 1 and dy1 == 1 then DoAction( "attack_1b", 1 - nCurDir ) nWait = 0
	elseif dx == 2 and dy == 0 then DoAction( "move_x", 0 )
    elseif dx == -2 and dy == 0 then DoAction( "move_x", 1 )
    elseif dx == 1 and dy == 1 then DoAction( "move_up", 0 )
    elseif dx == -1 and dy == 1 then DoAction( "move_up", 1 )
    elseif dx == 1 and dy == -1 then DoAction( "move_down", 0 )
    elseif dx == -1 and dy == -1 then DoAction( "move_down", 1 )
	end
   end
  end
  Wait( "stand", nWait )
 end
end

function AIFunc_PlayerTracer_SpecialAction( ai, pawn )
 local szCurState = ""
 local nCurDir = 0
 local nxtX = 0
 local nxtY = 0
 local target = nil
 nxtX, nxtY, target = ai:FindTarget( target )
 if not target then return end

 local function Wait( szState, nDelay, szNewState, nDir )
  if szState then
   while szCurState ~= szState do
    szCurState, nCurDir = coroutine.yield()
   end
  end
  if nDelay then
   for i = 1, nDelay, 1 do
    szCurState, nCurDir = coroutine.yield()
   end
  end
  if szNewState then
   szCurState, nCurDir = coroutine.yield( szNewState, nDir )
  end
 end
 local function DoAction( szNewState, nDir )
   szCurState, nCurDir = coroutine.yield( szNewState, nDir )
 end
 Wait( "stand" )

 for i = 1, 5, 1 do
  local x0 = pawn:GetToX()
  local y0 = pawn:GetToY()
  local x1 = target:GetToX()
  local y1 = target:GetToY()
  local dx = math.abs( x1 - x0 )
  local dy = math.abs( y1 - y0 )
  if dx == 2 and dy == 0 or dx == 1 and dy == 1 then
   if i <= RandInt( 1, 5 ) then
    local ofs = { { 2, 0, "move_x_flash", 0 }, { -2, 0, "move_x_flash", 1 },
	{ 1, 1, "move_up_flash", 0 }, { -1, 1, "move_up_flash", 1 },
	{ -1, 1, "move_down_flash", 0 }, { -1, -1, "move_down_flash", 1 } }
	local bFound = false
	for j = #ofs, 1, -1 do
	 local n = RandInt( 0, j ) + 1
	 if PawnCanMoveTo( pawn, x0 + ofs[n][1], y0 + ofs[n][2] ) then
	  bFound = true
	  DoAction( ofs[n][3], ofs[n][4] )
	  break
	 end
	 ofs[n] = ofs[j]
	end
	if not bFound then break end
   else
    break
   end
  else
   if nxtX >= 0 then
    dx = nxtX - x0
    dy = nxtY - y0
    if dx == 2 and dy == 0 then DoAction( "move_x_flash", 0 ) end
    if dx == -2 and dy == 0 then DoAction( "move_x_flash", 1 ) end
    if dx == 1 and dy == 1 then DoAction( "move_up_flash", 0 ) end
    if dx == -1 and dy == 1 then DoAction( "move_up_flash", 1 ) end
    if dx == 1 and dy == -1 then DoAction( "move_down_flash", 0 ) end
    if dx == -1 and dy == -1 then DoAction( "move_down_flash", 1 ) end
   else
    break
   end
  end
  nxtX, nxtY, target = ai:FindTarget( target )
  if not target then break end
 end
 Wait( "stand", 0 )
 GetMasterLevel():BlackOut( 48, 24 )
 Wait( nil, 24 )
end


function AIFunc_PlayerTracer_SpecialAction_Knife_Throw( ai, pawn )
 local szCurState = ""
 local nCurDir = 0
 local nxtX = 0
 local nxtY = 0
 local target = nil
 nxtX, nxtY, target = ai:FindTarget( target, 1 )
 if not target then return end

 local function Wait( szState, nDelay, szNewState, nDir )
  if szState then
   while szCurState ~= szState do
    szCurState, nCurDir = coroutine.yield()
   end
  end
  if nDelay then
   for i = 1, nDelay, 1 do
    szCurState, nCurDir = coroutine.yield()
   end
  end
  if szNewState then
   szCurState, nCurDir = coroutine.yield( szNewState, nDir )
  end
 end
 local function DoAction( szNewState, nDir )
   szCurState, nCurDir = coroutine.yield( szNewState, nDir )
 end
 Wait( "stand" )

 local n1 = 3
 local i = 0
 while true do
  i = i + 1
  if i > 5 then break end
  local x0 = pawn:GetToX()
  local y0 = pawn:GetToY()
  local x1 = target:GetToX()
  local y1 = target:GetToY()
  local x2 = target:GetPosX()
  local y2 = target:GetPosY()
  local dx = math.abs( x1 - x0 )
  local dy = math.abs( y1 - y0 )

  if nxtX >= 0 then
   local b = false
   if nxtY == y1 then
    b = true
	local ofsX = nxtX > x0 and -2 or 2
	for x = nxtX, x1 - ofsX, ofsX do
	 b = GetCurLevel():GetPawnByGrid( x, nxtY ) == target or PawnCanMoveTo( pawn, x, nxtY )
	 if not b then break end
	end
   end
   
   if b then
    local ofsX = x1 < x0 and 2 or -2
    if dy == 0 then
     if PawnCanMoveTo( pawn, x0 + ofsX, y0 ) then
	  nxtX = x0 + ofsX
	 end
    elseif dy == 1 then
     local ofsX = x1 < x0 and 2 or -2
     if PawnCanMoveTo( pawn, nxtX + ofsX, nxtY ) then
	  nxtX = nxtX + ofsX
	 end
    end
   end
   
   if b and dy == 0 and dx >= ( i >= 5 and 4 or 6 ) and i >= RandInt( 2, 5 ) then
    GetMasterLevel():BlackOut( 40, 20 )
    Wait( nil, 20 )
    DoAction( "throw", x1 < x0 and 1 or 0 )
	PlayerPickUp( "data/pickups/knife_throw.pf", pawn )
	i = 2
	n1 = n1 - 1
	if n1 == 0 then return end
	goto continue
   end
   if i >= RandInt( 3, 5 ) then
    if dx == 2 and dy == 0 or dx == 1 and dy == 1 then break end
   end
   
   dx = nxtX - x0
   dy = nxtY - y0
   if dx == 2 and dy == 0 then DoAction( "move_x_flash", 0 ) end
   if dx == -2 and dy == 0 then DoAction( "move_x_flash", 1 ) end
   if dx == 1 and dy == 1 then DoAction( "move_up_flash", 0 ) end
   if dx == -1 and dy == 1 then DoAction( "move_up_flash", 1 ) end
   if dx == 1 and dy == -1 then DoAction( "move_down_flash", 0 ) end
   if dx == -1 and dy == -1 then DoAction( "move_down_flash", 1 ) end
  else
   break
  end
  ::continue::
  nxtX, nxtY, target = ai:FindTarget( target, 1 )
  if not target then break end
 end
 Wait( "stand", 0 )
 GetMasterLevel():BlackOut( 48, 24 )
 Wait( nil, 24 )
end


function AIFunc_PlayerTracer_Action_Iron_Bar( ai, pawn )
 local szCurState = ""
 local nCurDir = 0
 local nxtX = 0
 local nxtY = 0
 local target = nil

 local function Wait( szState, nDelay, szNewState, nDir, nType )
  if szState then
   while szCurState ~= szState do
    szCurState, nCurDir = coroutine.yield()
   end
  end
  if nDelay then
   for i = 1, nDelay, 1 do
    szCurState, nCurDir = coroutine.yield()
   end
  end
  if szNewState or nType then
   szCurState, nCurDir = coroutine.yield( szNewState, nDir, nType )
  end
 end
 local function DoAction( szNewState, nDir )
   szCurState, nCurDir = coroutine.yield( szNewState, nDir, nType )
 end

 local function Func1( n )
  local b0 = false
  repeat
   local b = false

   local x0 = pawn:GetToX()
   local y0 = pawn:GetToY()
   local x1 = target:GetToX()
   local y1 = target:GetToY()
   local dx1 = x1 - x0
   local dy1 = y1 - y0
   local x2 = target:GetPosX()
   local y2 = target:GetPosY()
   local dx2 = x2 - x0
   local dy2 = y2 - y0

   local str_atk_1 = ( szCurState == "atk_1a" or szCurState == "atk_2a" ) and "atk_1b" or "atk_1a"
   local str_atk_2 = ( szCurState == "atk_1a" or szCurState == "atk_2a" ) and "atk_2b" or "atk_2a"

   if nCurDir > 0 then
    dx1 = -dx1
    dx2 = -dx2
   end
  
   local b1 = PawnCanMoveTo( pawn, pawn:GetToX() + ( nCurDir > 0 and -2 or 2 ), pawn:GetToY() )
   local b2 = PawnCanMoveTo( pawn, pawn:GetToX() + ( nCurDir > 0 and 2 or -2 ), pawn:GetToY() )
   local b3 = PawnCanMoveTo( pawn, pawn:GetToX() + ( nCurDir > 0 and 4 or -4 ), pawn:GetToY() )

   if dx1 == 1 and dy1 == 1 or dx2 == 1 and dy2 == 1 then DoAction( "atk_3", nCurDir ) n = 3 b = true goto continue end
   if dx1 == 1 and dy1 == -1 or dx2 == 1 and dy2 == -1 then DoAction( "atk_3", nCurDir ) n = 3 b = true goto continue end
   if n >= 3 then goto continue end
   if dx1 == 2 and dy1 == 0 or dx2 == 2 and dy2 == 0 then DoAction( n <= 2 and str_atk_1 or "atk_3", nCurDir ) n = ( n == 1 and 2 or n ) b = true goto continue end
   if dx1 < 0 and dy1 == 0 or dx2 < 0 and dy2 == 0 and n == 1
    or dx1 < 0 and dy1 == 1
    or dx1 < 0 and dy1 == -1 then
    if n == 1 then DoAction( "atk_5", 1 - nCurDir ) n = 2 b = true goto continue
    elseif b1 and dx1 >= -2 then DoAction( str_atk_2, nCurDir ) n = 2 b = true goto continue
    elseif b2 and( b3 or dx1 < -2 or dy2 == 0 ) then DoAction( "atk_5a", 1 - nCurDir ) n = 2 b = true goto continue
    else goto continue end
   end

   if b1 and ( dx2 == 4 and dy2 == 0 or dx1 > 2 and dy1 >= -1 and dy1 <= 1 ) then DoAction( str_atk_2, nCurDir ) n = 2 b = true end
   ::continue::
   b0 = b0 or b
  until not b or not ai:IsValidTarget( target )

  return b0
 end
 Wait( "stand" )

 while true do
  nxtX, nxtY, target = ai:FindTarget( target, 1 )
  local bAction = false
  if target then
   local n = 1
   repeat
    local bAction0 = Func1( n ) n = 2
	bAction = bAction or bAction0
	if bAction0 then Wait( "recover", 0 ) end
   until not bAction0 or not ai:IsValidTarget( target )
  end

  if not bAction then
   if nxtX >= 0 and PawnCanMoveTo( pawn, nxtX, nxtY ) then
    local x0 = pawn:GetToX()
    local y0 = pawn:GetToY()
    local dx = nxtX - x0
    local dy = nxtY - y0

    if dx == 2 and dy == 0 then DoAction( "move_x", 0 )
    elseif dx == -2 and dy == 0 then DoAction( "move_x", 1 )
    elseif dx == 1 and dy == 1 then DoAction( "move_up", 0 )
    elseif dx == -1 and dy == 1 then DoAction( "move_up", 1 )
    elseif dx == 1 and dy == -1 then DoAction( "move_down", 0 )
    elseif dx == -1 and dy == -1 then DoAction( "move_down", 1 )
	end

    Wait( "stand", 0 )
    nxtX, nxtY, target = ai:FindTarget( target, 1 )
	if target then
     local n = 1
     repeat
	  local bAction0 = Func1( n ) n = 2
	  if bAction0 then Wait( "recover", 0 ) end
	 until not bAction0 or not ai:IsValidTarget( target )
    end
   end
  end
  Wait( "stand", 0, nil, nil, 1 )
  Wait( "stand", 30 )
 end
end

function AIFunc_PlayerTracer_SpecialAction_Iron_Bar( ai, pawn )
 local szCurState = ""
 local nCurDir = 0
 local nxtX = 0
 local nxtY = 0
 local target = nil
 nxtX, nxtY, target = ai:FindTarget( target, 1 )
 if not target then return end

 local function Wait( szState, nDelay, szNewState, nDir )
  if szState then
   while szCurState ~= szState do
    szCurState, nCurDir = coroutine.yield()
   end
  end
  if nDelay then
   for i = 1, nDelay, 1 do
    szCurState, nCurDir = coroutine.yield()
   end
  end
  if szNewState then
   szCurState, nCurDir = coroutine.yield( szNewState, nDir )
  end
 end
 local function DoAction( szNewState, nDir )
   szCurState, nCurDir = coroutine.yield( szNewState, nDir )
 end
 Wait( "stand" )

 local i = 0
 local function Func1()
  i = 0
  GetMasterLevel():BlackOut( 48, 24 )
  Wait( nil, 24 )
  local x0 = pawn:GetToX()
  local y0 = pawn:GetToY()
  local x1 = target:GetToX()
  local y1 = target:GetToY()
  local dx1 = x1 - x0
  local dy1 = y1 - y0
  local x2 = target:GetPosX()
  local y2 = target:GetPosY()
  local dx2 = x2 - x0
  local dy2 = y2 - y0

  if dx1 == 2 and dy1 == 0 then Wait( nil, 3 ) DoAction( "atk_3", 0 ) return end
  if dx1 == -2 and dy1 == 0 then Wait( nil, 3 ) DoAction( "atk_3", 1 ) return end
  local nDir = dx1 > 0 and 0 or 1
  Wait( nil, 11 )
  local b1 = PawnCanMoveTo( pawn, pawn:GetToX() + ( nDir > 0 and -2 or 2 ), pawn:GetToY() )
  if not b1 or RandInt( 0, 2 ) == 0 then
   if dx1 == 1 and dy1 == 1 or dx1 == 1 and dy1 == -1 then Wait( nil, 3 ) DoAction( "atk_3", 0 ) return end
   if dx1 == -1 and dy1 == 1 or dx1 == -1 and dy1 == -1 then Wait( nil, 3 ) DoAction( "atk_3", 1 ) return end
   if dx2 == 1 and dy2 == 1 or dx2 == 1 and dy2 == -1 then Wait( nil, 3 ) DoAction( "atk_3", 0 ) return end
   if dx2 == -1 and dy2 == 1 or dx2 == -1 and dy2 == -1 then Wait( nil, 3 ) DoAction( "atk_3", 1 ) return end
  end
  
  if ( dy1 >= -1 and dy1 <= 1 ) and ( dx1 >= -4 and dx1 <= 4 ) then
   if b1 then DoAction( "atk_2a", nDir ) DoAction( "atk_3", nDir ) DoAction( "atk_5a", 1 - nDir ) return end
  end

 end

 for j = 1, 15, 1 do
  i = i + 1
  local x0 = pawn:GetToX()
  local y0 = pawn:GetToY()
  local x1 = target:GetToX()
  local y1 = target:GetToY()
  local dx = math.abs( x1 - x0 )
  local dy = math.abs( y1 - y0 )
  if dx == 2 and dy == 0 or dx == 1 and dy == 1 then
   if i <= RandInt( 2, 6 ) then
    local ofs = { { 2, 0, "move_x_flash", 0 }, { -2, 0, "move_x_flash", 1 },
	{ 1, 1, "move_up_flash", 0 }, { -1, 1, "move_up_flash", 1 },
	{ -1, 1, "move_down_flash", 0 }, { -1, -1, "move_down_flash", 1 } }
	local bFound = false
	for j = #ofs, 1, -1 do
	 local n = RandInt( 0, j ) + 1
	 if PawnCanMoveTo( pawn, x0 + ofs[n][1], y0 + ofs[n][2] ) then
	  bFound = true
	  DoAction( ofs[n][3], ofs[n][4] )
	  break
	 end
	 ofs[n] = ofs[j]
	end
	if not bFound then Func1() end
   else Func1() end
  else
   if nxtX >= 0 then
    dx = nxtX - x0
    dy = nxtY - y0
    if dx == 2 and dy == 0 then DoAction( "move_x_flash", 0 ) end
    if dx == -2 and dy == 0 then DoAction( "move_x_flash", 1 ) end
    if dx == 1 and dy == 1 then DoAction( "move_up_flash", 0 ) end
    if dx == -1 and dy == 1 then DoAction( "move_up_flash", 1 ) end
    if dx == 1 and dy == -1 then DoAction( "move_down_flash", 0 ) end
    if dx == -1 and dy == -1 then DoAction( "move_down_flash", 1 ) end
   else
    break
   end
  end
  nxtX, nxtY, target = ai:FindTarget( target, 1 )
  if not target then break end
 end
 GetMasterLevel():BlackOut( 60, 30 )
 Wait( "stand", 30 )
end

function AIFunc_PlayerTracer_Action_Revolver( ai, pawn, szCurState, nCurDir )
 local player = GetPlayer()
 if player:IsHidden() then return end
 
 local x0 = pawn:GetToX()
 local y0 = pawn:GetToY()
 local x1 = player:GetToX()
 local y1 = player:GetToY()
 local x2 = player:GetPosX()
 local y2 = player:GetPosY()
 local dx = x1 - x0
 local dy = y1 - y0
 local dx2 = x2 - x0
 local dy2 = y2 - y0
 local ofsX = 0
 local ofsY = 0
 local l = 0
 if dy == 0 then ofsX = dx > 0 and 2 or -2 l = math.abs( dx ) / 2
 elseif dx == dy then ofsX = dx > 0 and 1 or -1 ofsY = ofsX l = math.abs( dx )
 elseif dx == -dy then ofsX = dx > 0 and 1 or -1 ofsY = -ofsX l = math.abs( dx )
 elseif dy2 == 0 then ofsX = dx2 > 0 and 2 or -2 l = math.abs( dx2 ) / 2
 elseif dx2 == dy then ofsX = dx2 > 0 and 1 or -1 ofsY = ofsX l = math.abs( dx2 )
 elseif dx2 == -dy then ofsX = dx2 > 0 and 1 or -1 ofsY = -ofsX l = math.abs( dx2 )
 else return end
 if l <= 1 then return end
 for i = 1, l - 1, 1 do
  x0 = x0 + ofsX
  y0 = y0 + ofsY
  if not BulletCanPass( player, x0, y0 ) then return end
 end

 if ofsX == 2 and ofsY == 0 then return "shoot_x", 0 end
 if ofsX == -2 and ofsY == 0 then return "shoot_x", 1 end
 if ofsX == 1 and ofsY == 1 then return "shoot_up", 0 end
 if ofsX == -1 and ofsY == 1 then return "shoot_up", 1 end
 if ofsX == 1 and ofsY == -1 then return "shoot_down", 0 end
 if ofsX == -1 and ofsY == -1 then return "shoot_down", 1 end
end


function AIFunc_PlayerTracer_SpecialAction_Valve( ai, pawn )
 local szCurState = ""
 local nCurDir = 0
 local nxtX = 0
 local nxtY = 0

 local function Wait( szState, nDelay, szNewState, nDir )
  if szState then
   while szCurState ~= szState do
    szCurState, nCurDir = coroutine.yield()
   end
  end
  if nDelay then
   for i = 1, nDelay, 1 do
    szCurState, nCurDir = coroutine.yield()
   end
  end
  if szNewState then
   szCurState, nCurDir = coroutine.yield( szNewState, nDir )
  end
 end
 local function DoAction( szNewState, nDir )
   szCurState, nCurDir = coroutine.yield( szNewState, nDir )
 end

 while true do
  Wait( "stand" )
  local x0 = pawn:GetToX()
  local y0 = pawn:GetToY()
  nxtX, nxtY = ai:FindTarget1()
  if not nxtX then Wait( nil, 1 )
  else
   if nxtX > x0 and nxtY == y0 then DoAction( "move_x_ready", 0 ) end
   if nxtX < x0 and nxtY == y0 then DoAction( "move_x_ready", 1 ) end
   if nxtX > x0 and nxtY > y0 then DoAction( "move_up_ready", 0 ) end
   if nxtX < x0 and nxtY > y0 then DoAction( "move_up_ready", 1 ) end
   if nxtX > x0 and nxtY < y0 then DoAction( "move_down_ready", 0 ) end
   if nxtX < x0 and nxtY < y0 then DoAction( "move_down_ready", 1 ) end
  end
 end
end

function PawnAIVortex_Wait( ai, pawn )
 while not pawn:IsKilled() do
  coroutine.yield()
 end
end

function PawnAIVortex_Wait1( ai, pawn )
 while ai:GetAliveSpawns() > 0 do
  coroutine.yield()
 end
end