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