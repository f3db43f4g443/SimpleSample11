function Env_CommonEvent_0( env, level )
 while true do
  local params = { coroutine.yield( 1 ) }
  local szType = params[1]
  if szType == "player_move_begin" then
   if not GetMasterLevel():IsScenario() then
    env:ApplyForce( -2, 1, 0, 120, 8, 2 )
    env:ApplyForce( -1, 1, 0, 120, 8, 2 )
   end
  elseif szType == "player_move_end" then
   if not GetMasterLevel():IsScenario() then
    env:ApplyForce( -2, 1, 0, -120, 8, 2 )
    env:ApplyForce( -1, 1, 0, -120, 8, 2 )
   end
  elseif szType == "enemy_move_begin" then
   if not GetMasterLevel():IsScenario() then
    env:ApplyForce( -2, 1, 0, 240, 8, 2 )
    env:ApplyForce( -1, 1, 0, 240, 8, 2 )
   end
  elseif szType == "enemy_move_end" then
   if not GetMasterLevel():IsScenario() then
    env:ApplyForce( -2, 1, 0, -240, 8, 2 )
    env:ApplyForce( -1, 1, 0, -240, 8, 2 )
   end
  end
 end
end

function Env_CommonEvent_1( env, level )
 Env_CommonEvent_1_Ex( -120, -240, 8, env, level )
end

function Env_CommonEvent_1_Ex( f1, f2, t, env, level )
 local x0 = env:GetCtrlPointOrigX( -2 )
 local x1 = env:GetCtrlPointOrigX( -1 )
 while true do
  local params = { coroutine.yield( 1 ) }
  local szType = params[1]
  local pawn = params[2]
  if szType == "player_move_begin" or szType == "player_move_end" then
   if not GetMasterLevel():IsScenario() then
    local x = ( pawn:GetPosX() + 1 ) * LEVEL_GRID_SIZE_X
    local k = ( x - x0 ) / ( x1 - x0 )
    env:ApplyForce( -2, 1, 0, f1 * ( 1 - k ), t, 2 )
    env:ApplyForce( -1, 1, 0, f1 * k, t, 2 )
   end
  elseif szType == "enemy_move_begin" or szType == "enemy_move_end" then
   if not GetMasterLevel():IsScenario() then
    local x = ( pawn:GetPosX() + 1 ) * LEVEL_GRID_SIZE_X
    local k = ( x - x0 ) / ( x1 - x0 )
    env:ApplyForce( -2, 1, 0, f2 * ( 1 - k ), t, 2 )
    env:ApplyForce( -1, 1, 0, f2 * k, t, 2 )
   end
  end
 end
end

function Env_CommonEvent_2( env, level )
 Env_CommonEvent_2_Ex( -5, env, level )
end

function Env_CommonEvent_2_Ex( f, env, level )
 Env_CommonEvent_2_Ex2( f, 0, env, level )
end

function Env_CommonEvent_2_Ex2( f, f1, env, level )
 local x0 = env:GetCtrlPointOrigX( 0 )
 local y0 = env:GetCtrlPointOrigY( 0 )
 while true do
  local params = { coroutine.yield( 1 ) }
  local szType = params[1]
  local player = params[2]
  if szType == "player_action" then
   if not GetMasterLevel():IsScenario() then
    local x = ( player:GetToX() + 1 ) * LEVEL_GRID_SIZE_X
    local y = ( player:GetToY() + 0.5 ) * LEVEL_GRID_SIZE_Y
    env:ApplyForce( -2, 1, ( x - x0 ) * f, ( y - y0 ) * f, 12, 0 )
    env:ApplyForce( -1, 1, ( x - x0 ) * f, ( y - y0 ) * f, 12, 0 )
   end
  end
  if szType == "player_move_begin" then
   if not GetMasterLevel():IsScenario() then
    local x = ( player:GetToX() + 1 ) * LEVEL_GRID_SIZE_X
    local y = ( player:GetToY() + 0.5 ) * LEVEL_GRID_SIZE_Y
    env:ApplyForce( -2, 1, ( x - x0 ) * f, ( y - y0 ) * f, 24, 0 )
    env:ApplyForce( -1, 1, ( x - x0 ) * f, ( y - y0 ) * f, 24, 0 )
	if f1 ~= 0 then
     env:ApplyForce( -2, 1, 0, f1, 8, 2 )
     env:ApplyForce( -1, 1, 0, f1, 8, 2 )
	end
   end
  elseif szType == "player_move_end" then
   if not GetMasterLevel():IsScenario() then
    if f1 ~= 0 then
     env:ApplyForce( -2, 1, 0, -f1, 8, 2 )
     env:ApplyForce( -1, 1, 0, -f1, 8, 2 )
	end
   end
  end
 end
end

function Env_CommonEvent_3( env, level )
 local x0 = ( env:GetCtrlPointOrigX( -2 ) + env:GetCtrlPointOrigX( -1 ) ) / 2
 local y0 = ( env:GetCtrlPointOrigY( -2 ) + env:GetCtrlPointOrigY( -1 ) ) / 2
 local x1 = env:GetCtrlPointOrigX( -2 )
 local x2 = env:GetCtrlPointOrigX( -1 )
 while true do
  local params = { coroutine.yield( 1 ) }
  local szType = params[1]
  if szType == "player_action" then
   local player = params[2]
   local hidden = player:IsHidden()
   if not GetMasterLevel():IsScenario() then
    local x = ( player:GetToX() + 1 ) * LEVEL_GRID_SIZE_X
    local y = ( player:GetToY() + 0.5 ) * LEVEL_GRID_SIZE_Y
    env:ApplyForce( -2, 1, ( x - x0 ) * -5, ( y - y0 ) * -5, 12, 0 )
    env:ApplyForce( -1, 1, ( x - x0 ) * -5, ( y - y0 ) * -5, 12, 0 )
   end
  elseif szType == "player_move_begin" then
   local player = params[2]
   local hidden = player:IsHidden()
   if not GetMasterLevel():IsScenario() then
    local x = ( player:GetToX() + 1 ) * LEVEL_GRID_SIZE_X
    local y = ( player:GetToY() + 0.5 ) * LEVEL_GRID_SIZE_Y
    env:ApplyForce( -2, 1, ( x - x0 ) * -5, ( y - y0 ) * -5, 24, 0 )
    env:ApplyForce( -1, 1, ( x - x0 ) * -5, ( y - y0 ) * -5, 24, 0 )
	if not hidden then
     env:ApplyForce( -2, 1, 0, -12000, 6, 2 )
     env:ApplyForce( -1, 1, 0, -12000, 6, 2 )
	end
   end
  elseif szType == "player_move_end" then
   local player = params[2]
   local hidden = player:IsHidden()
   if not GetMasterLevel():IsScenario() then
	if not hidden then
     env:ApplyForce( -2, 1, 0, 12000, 6, 2 )
     env:ApplyForce( -1, 1, 0, 12000, 6, 2 )
	end
   end
  elseif szType == "alert" then
   local player = GetPlayer()
   local hidden = player:IsHidden()
   if not GetMasterLevel():IsScenario() then
	if hidden then
     local x = ( params[3] + 1 ) * LEVEL_GRID_SIZE_X
     local k = ( x - x1 ) / ( x2 - x1 )

     env:ApplyForce( -2, 1, 0, -12000 * ( 1 - k ), 24, 2 )
     env:ApplyForce( -1, 1, 0, -12000 * k, 24, 2 )
     env:ApplyForce( -2, 1, 0, 12000 * ( 1 - k ), 12, 1 )
     env:ApplyForce( -1, 1, 0, 12000 * k, 12, 1 )
	end
   end
  end
 end
end