function Interaction_Common()
 coroutine.yield()
 while true do
  if IsKeyDown( VK_ESCAPE ) then return end
  coroutine.yield()
 end
end

function Interaction_Video( ui )
 local selected = ui:FindChildEntity( "selected" )
 local texts = {}
 local nSelected = 0
 local unlocked = {}
 for i = 1, 5, 1 do
  texts[i] = ui:FindChildEntity( "text_" .. tostring( i ) )
  unlocked[i] = EvaluateKeyInt( "%video_unlocked_" .. tostring( i ) ) > 0
  if unlocked[i] then
   local item = g_videos[i]
   texts[i]:Set( item.strName, 0 )
   if nSelected == 0 then
    nSelected = i
   end
  else
   texts[i]:Set( "?????????????", 0 )
  end
 end
 if nSelected == 0 then
  selected:SetVisible( false )
 end
 coroutine.yield()

 while true do
  if IsKeyDown( VK_ESCAPE ) then return end
  if nSelected > 0 then
   if IsInputDown( 1 ) then
    for i = 1, 5, 1 do
     nSelected = nSelected + 1
     if nSelected > 5 then
      nSelected = 1
     end
     if unlocked[nSelected] then
      break
     end
    end
   end
   if IsInputDown( 3 ) then
    for i = 1, 5, 1 do
     nSelected = nSelected - 1
     if nSelected < 1 then
      nSelected = 5
     end
     if unlocked[nSelected] then
      break
     end
    end
   end
   SetPosition( selected, 0, 288 - nSelected * 96 )
  end

  if IsKeyDown( VK_RETURN ) or IsKeyDown( string.byte( " " ) ) or IsInputDown( 4 ) then
   if nSelected > 0 then
    local item = g_videos[nSelected]
    RunScenario( item.funcScenario )
   end
   return
  end
  coroutine.yield()
 end
end

function Interaction_Bookshelf( ui, pawn )
 local p1 = ui:FindChildEntity( "1" )
 local a = { p1:FindChildEntity( "a1" ), p1:FindChildEntity( "a2" ), p1:FindChildEntity( "a3" ) }
 local b = { p1:FindChildEntity( "b1" ), p1:FindChildEntity( "b2" ), p1:FindChildEntity( "b3" ) }
 local books0 = ui:FindChildEntity( "books" )
 local books = { { books0:FindChildEntity( "a1" ), books0:FindChildEntity( "a2" ), books0:FindChildEntity( "a3" ),
  books0:FindChildEntity( "a4" ), books0:FindChildEntity( "a5" ), },
  { books0:FindChildEntity( "b1" ), books0:FindChildEntity( "b2" ), books0:FindChildEntity( "b3" ),
  books0:FindChildEntity( "b4" ), books0:FindChildEntity( "b5" ), books0:FindChildEntity( "b6" ), },
  { books0:FindChildEntity( "c1" ), books0:FindChildEntity( "c2" ), books0:FindChildEntity( "c3" ),
  books0:FindChildEntity( "c4" ), books0:FindChildEntity( "c5" ), books0:FindChildEntity( "c6" ), },
 }
 local lighta = { ui:FindChildEntity( "l1a" ), ui:FindChildEntity( "l2a" ), ui:FindChildEntity( "l3a" ) }
 local lightb = { ui:FindChildEntity( "l1b" ), ui:FindChildEntity( "l2b" ), ui:FindChildEntity( "l3b" ) }

 local p2 = ui:FindChildEntity( "2" )
 local handle1 = p2:FindChildEntity( "handle1" )
 local handle2 = ui:FindChildEntity( "handle2" )
 local handle0 = ui:FindChildEntity( "handle0" )
 local cursor = ui:FindChildEntity( "cursor" )
 local bookSelect = ui:FindChildEntity( "book_select" )
 local OKEft = ui:FindChildEntity( "ok" )
 
 for i = 1, 3, 1 do
  local k = EvaluateKeyInt( "_BOOKSHELF_TAPE_" .. tostring( i ) )
  a[i]:SetVisible( k == 0 )
  b[i]:SetVisible( k > 0 )
 end
 local kHandle = EvaluateKeyInt( "_BOOKSHELF_HANDLE" )
 handle1:SetVisible( kHandle > 0 )
 handle2:SetVisible( kHandle > 0 )
 handle0:SetVisible( kHandle == 0 )
 bookSelect:SetVisible( false )
 OKEft:SetVisible( false )

 local ScenarioFunc = function()
  Delay( 30 )
  PlaySoundEffect( "activate" )
  Delay( 10 )
  GetMasterLevel():BlackOut( 60, 5 )
  GetCurLevel():SpawnPawn( 0, pawn:GetPosX() - 1, pawn:GetPosY() + 1, 0 )
  GetCurLevel():RemovePawn( pawn )
 end
 
 local bookSpace = {
  { 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0 },
  { 1, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1 },
  { 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0 },
 }
 local bookState = { { 10, 9, 0, 11, 2 }, { 11, 5, 12, 1, 0, 2 }, { 7, 4, 9, 0, 1, 5 } }
 local bookSolution = { { 1, 2, 9, 10, 11 }, { 1, 2, 3, 4, 5, 10 }, { 1, 2, 3, 6, 8, 9 } }
 local bookMissing = { 3, 5, 4 }
 for i = 1, 3, 1 do
  for j = 1, #bookState[i], 1 do
   local kState = EvaluateKeyInt( "_BOOKSHELF_BOOK_STATE_" .. tostring( i ).. "_" .. tostring( j ) )
   if kState > 0 then
    bookState[i][j] = kState
   end

   if bookState[i][j] == 0 then
    books[i][j]:SetVisible( false )
   else
    SetPosition( books[i][j], ( bookState[i][j] - 7 ) * 42, 0 )
   end
  end
 end

 local RefreshLight = function()
  local bOK = true
  for i = 1, 3, 1 do
   local bOK1 = true
   local bOK2 = true
   for j = 1, #bookState[i], 1 do
    if bookState[i][j] == 0 then
     bOK1 = false
    end
    if bookState[i][j] ~= bookSolution[i][j] then
     bOK2 = false
	 bOK = false
    end
   end
   lighta[i]:SetVisible( bOK1 )
   lightb[i]:SetVisible( bOK2 )
  end
  return bOK
 end
 RefreshLight()

 local cursorX = 0
 local cursorY = 0
 local bOpen = false
 local nSelectedRow = 0
 local nSelectedBook = 0
 local handleRect = { 256, 32, 304, 96 }
 local tapeRect = { { -288, 160, 288, 200 }, { -288, -96, 288, -56 }, { -288, -352, 288, -312 } }

 coroutine.yield()
 while not IsKeyDown( VK_ESCAPE ) do
  if IsInput( 0 ) then
   cursorX = math.min( 320, cursorX + 4 )
   SetPosition( cursor, cursorX, cursorY )
  end
  if IsInput( 1 ) then
   cursorY = math.min( 384, cursorY + 4 )
   SetPosition( cursor, cursorX, cursorY )
  end
  if IsInput( 2 ) then
   cursorX = math.max( -320, cursorX - 4 )
   SetPosition( cursor, cursorX, cursorY )
  end
  if IsInput( 3 ) then
   cursorY = math.max( -384, cursorY - 4 )
   SetPosition( cursor, cursorX, cursorY )
  end

  if IsInputDown( 4 ) then
   if bOpen then
    local b0 = false
    for i = 1, 3, 1 do
     if cursorX >= tapeRect[i][1] and cursorY >= tapeRect[i][2] and cursorX <= tapeRect[i][3] and cursorY <= tapeRect[i][4] then
      if EvaluateKeyInt( "_BOOKSHELF_TAPE_" .. tostring( i ) ) == 0 and GetLabelKey( "_TAPE" ) > 0 then
       SetKeyInt( "_BOOKSHELF_TAPE_" .. tostring( i ), 1 )
       SetLabelKey( "_TAPE", GetLabelKey( "_TAPE" ) + 1 )
       a[i]:SetVisible( false )
       b[i]:SetVisible( true )
       b0 = true
       PlaySoundEffect( "btn" )
      end
     end
    end

    if not b0 then
     local n = 0
     if cursorY >= -320 and cursorY <= -96 then
      n = 3
     elseif cursorY >= -64 and cursorY <= 160 then
      n = 2
     elseif cursorY >= 192 then
      n = 1
     end
     local x = 1 + ( cursorX + 252 ) // 42
     if n > 0 and x >= 1 and x <= 12 and bookSpace[n][x] == 1 then
      local iBook = 0
      for i = 0, #bookState[n], 1 do
       if bookState[n][i] == x then
        iBook = i
        break
       end
      end
      
      if nSelectedRow == n then
	   if nSelectedBook ~= iBook then
	    local x1 = bookState[n][nSelectedBook]
	    bookState[n][nSelectedBook] = x
        SetPosition( books[n][nSelectedBook], ( x - 7 ) * 42, 0 )
		SetKeyInt( "_BOOKSHELF_BOOK_STATE_" .. tostring( n ).. "_" .. tostring( nSelectedBook ), x )
		if iBook > 0 then
		 bookState[n][iBook] = x1
         SetPosition( books[n][iBook], ( x1 - 7 ) * 42, 0 )
		 SetKeyInt( "_BOOKSHELF_BOOK_STATE_" .. tostring( n ).. "_" .. tostring( iBook ), x1 )
		end
	   end
	   nSelectedRow = 0
	   nSelectedBook = 0
       PlaySoundEffect( "btn" )
       bookSelect:SetVisible( false )
      else
	   nSelectedRow = 0
	   nSelectedBook = 0
       bookSelect:SetVisible( false )
       if iBook > 0 then
	    nSelectedRow = n
        nSelectedBook = iBook
        bookSelect:SetVisible( true )
		SetPosition( bookSelect, ( x - 7 ) * 42, -320 + ( 3 - n ) * 256 )
        PlaySoundEffect( "btn" )
       elseif GetLabelKey( "_BOOK_" .. tostring( n ) ) == 1 then
	    iBook = bookMissing[n]
	    bookState[n][iBook] = x
        books[n][iBook]:SetVisible( true )
        SetPosition( books[n][iBook], ( x - 7 ) * 42, 0 )
		SetKeyInt( "_BOOKSHELF_BOOK_STATE_" .. tostring( n ).. "_" .. tostring( iBook ), x )
		SetLabelKey( "_BOOK_" .. tostring( n ), 2 )
        PlaySoundEffect( "btn" )
       else
        PlaySoundEffect( "btn_error" )
       end
      end

	  local bOK = RefreshLight()
	  if bOK then
       bookSelect:SetVisible( false )
       p2:SetVisible( true )
       OKEft:SetVisible( true )
	   Delay( 60 )
       SetKeyInt( "%bookshelf_ok", 1 )
	   RunScenario( ScenarioFunc )
	   return
	  end

	 else
      PlaySoundEffect( "btn_error" )
     end
    end
   else
    local bOK = false
    if kHandle == 0 and GetLabelKey( "_HANDLE" ) > 0 then
     if cursorX >= handleRect[1] and cursorY >= handleRect[2] and cursorX <= handleRect[3] and cursorY <= handleRect[4] then
      kHandle = 1
      SetKeyInt( "_BOOKSHELF_HANDLE", 1 )
      SetLabelKey( "_HANDLE", 2 )
      handle1:SetVisible( kHandle > 0 )
      handle2:SetVisible( kHandle > 0 )
      handle0:SetVisible( kHandle == 0 )
      PlaySoundEffect( "btn" )
	  bOK = true
     end
    end
	if not bOK then
     PlaySoundEffect( "btn_error" )
	end
   end
  end

  if IsInputDown( 5 ) and kHandle == 1 then
   if bOpen then
    bOpen = false
	nSelectedRow = 0
    nSelectedBook = 0
    bookSelect:SetVisible( false )
    p2:SetVisible( true )
   else
    bOpen = true
    p2:SetVisible( false )
   end
  end
  
  coroutine.yield()
 end
end

function Interaction_DoorLock( ui, pwdKey )
 local ok = ui:FindChildEntity( "ok" )
 local error = ui:FindChildEntity( "error" )
 local p1 = ui:FindChildEntity( "1" )
 ok:SetVisible( false )
 error:SetVisible( false )
 p1:SetVisible( false )
 local keys = {}
 local ofs = { { 0, -128 }, { -96, 160 }, { 0, 160 }, { 96, 160 }, { -96, 64 }, { 0, 64 }, { 96, 64 }, { -96, -32 }, { 0, -32 }, { 96, -32 } }
 for i = 1, 10, 1 do
  keys[i] = string.byte( tostring( i - 1 ) )
 end

 pwdKey = pwdKey or "$pwd"
 local str = ""
 local pwd = EvaluateKeyString( pwdKey )
 while true do
  coroutine.yield()
  if IsKeyDown( VK_ESCAPE ) then return end
  local n = 0
  for i = 1, 10, 1 do
   if IsKeyDown( keys[i] ) then
    str = str .. tostring( i - 1 )
    n = i
    break
   end
  end
  if n > 0 then
   PlaySoundEffect( "btn" )
   p1:SetVisible( true )
   SetPosition( p1, ofs[n][1], ofs[n][2] )
   Delay( 10 )
   p1:SetVisible( false )
  end
  if #str >= 6 then
   if str == pwd then
    PlaySoundEffect( "btn" )
    ok:SetVisible( true )
    Delay( 60 )
	return true
   else
    PlaySoundEffect( "btn_error" )
    error:SetVisible( true )
    Delay( 60 )
    error:SetVisible( false )
   end
   str = ""
  end
 end

 return false
end

function Init_Cinema_Seat()
 local level = GetCurLevel()
 for i = 1, 9, 1 do
  if EvaluateKeyInt( "_CINEMA_SEAT_" .. tostring( i ) ) > 0 then
   local x = EvaluateKeyInt( "_CINEMA_SEAT_X_" .. tostring( i ) )
   local y = EvaluateKeyInt( "_CINEMA_SEAT_Y_" .. tostring( i ) )
   local npc = level:GetPawnByName( tostring( i ) )
   npc:PlayStateForceMove( "", x, y, npc:GetCurDir() )
  end
 end
end

function Interaction_Cinema_Seat( ui, pawn )
 local npcs = {}
 local npcData = { { 6, 4, 5 }, { 6, 2, 1 }, { 9, 3, 1 }, { 10, 0, 1 }, { 14, 2, 2 }, { 8, 8, 2 }, { 4, 6, 2 }, { 15, 5, 3 }, { 10, 2, 4 } }
 local npcGrids = { { { 6, 4 }, { 5, 5 }, { 5, 1 }, { 10, 2 }, { 15, 1 }, { 14, 4 }, { 15, 5 } },
  { { 6, 2 }, { 4, 2 }, { 5, 1 } },
  { { 9, 3 }, { 10, 4 }, { 11, 3 }, { 10, 2 } },
  { { 10, 0 }, { 12, 0 }, { 8, 0 } },
  { { 14, 2 }, { 16, 2 }, { 4, 2 }, { 6, 2 } },
  { { 8, 8 }, { 12, 8 }, { 14, 8 }, { 6, 8 } },
  { { 4, 6 }, { 6, 6 }, { 10, 6 }, { 14, 6 }, { 16, 6 } },
  { { 15, 5 }, { 16, 6 }, { 10, 0 }, { 14, 4 } },
  { { 10, 2 }, { 9, 3 }, { 7, 5 }, { 6, 6 }, { 12, 0 } },
 }

 for i = 1, #npcData, 1 do
  local data = npcData[i]
  if EvaluateKeyInt( "_CINEMA_SEAT_" .. tostring( i ) ) > 0 then
   data[1] = EvaluateKeyInt( "_CINEMA_SEAT_X_" .. tostring( i ) )
   data[2] = EvaluateKeyInt( "_CINEMA_SEAT_Y_" .. tostring( i ) )
  end
  npcs[i] = ui:FindChildEntity( tostring( i ) )
  SetPosition( npcs[i], data[1] * 24 - 264, data[2] * 32 - 160 )
 end
 local eftX = ui:FindChildEntity( "x" )
 eftX:SetVisible( false )
 local cursorX = ui:FindChildEntity( "cursor_x" )
 local cursorY = ui:FindChildEntity( "cursor_y" )
 local cursorPos = { 0, 0 }
 local cursorRegion = { -264, -160, 264, 352 }

 coroutine.yield()
 while not IsKeyDown( VK_ESCAPE ) do
  if IsInput( 0 ) then
   cursorPos[1] = math.min( cursorRegion[3] - 4, cursorPos[1] + 4 )
   SetPosition( cursorX, cursorPos[1], 0 )
  end
  if IsInput( 1 ) then
   cursorPos[2] = math.min( cursorRegion[4] - 4, cursorPos[2] + 4 )
   SetPosition( cursorY, 0, cursorPos[2] )
  end
  if IsInput( 2 ) then
   cursorPos[1] = math.max( cursorRegion[1], cursorPos[1] - 4 )
   SetPosition( cursorX, cursorPos[1], 0 )
  end
  if IsInput( 3 ) then
   cursorPos[2] = math.max( cursorRegion[2], cursorPos[2] - 4 )
   SetPosition( cursorY, 0, cursorPos[2] )
  end

  if IsInputDown( 4 ) then
   local x = math.floor( ( cursorPos[1] + 2 - cursorRegion[1] ) / 24 )
   local y = math.floor( ( cursorPos[2] + 2 - cursorRegion[2] ) / 32 )
   if ( x + y ) % 2 == 1 then
    x = x - 1
   end
   
   eftX:SetVisible( true )
   SetPosition( eftX, x * 24 - 264, y * 32 - 160 )
   PlaySoundEffect( "electric1" )
   Delay( 20 )

   for i = 1, #npcData, 1 do
    local data = npcData[i]
	if data[1] == x and data[2] == y then
	 local grids = npcGrids[i]
	 local j0 = 0
	 local n = #grids
	 for j = 1, n, 1 do
	  if grids[j][1] == x and grids[j][2] == y then
	   j0 = j
	   break
	  end
	 end

	 for j = 1, n - 1, 1 do
	  local j1 = ( j0 + j - 1 ) % n + 1
	  local x1 = grids[j1][1]
	  local y1 = grids[j1][2]
	  local b = true
	  for k = 1, #npcData, 1 do
	   local data1 = npcData[k]
	   if data1[1] == x1 and data1[2] == y1 or data[3] == 5 and ( data1[1] == x1 + 1 or data1[1] == x1 - 1 ) and data1[2] == y1 + 1
	    or data1[3] == 5 and ( data1[1] == x1 + 1 or data1[1] == x1 - 1 ) and data1[2] == y1 - 1 then
	    b = false
		break
	   end
	  end
	  if b then
	   data[1] = x1
	   data[2] = y1
       SetPosition( npcs[i], x1 * 24 - 264, y1 * 32 - 160 )
       SetKeyInt( "_CINEMA_SEAT_" .. tostring( i ), 1 )
       SetKeyInt( "_CINEMA_SEAT_X_" .. tostring( i ), x1 )
       SetKeyInt( "_CINEMA_SEAT_Y_" .. tostring( i ), y1 )
	   break
	  end
	 end

	 break
	end
   end

   Delay( 20 )
   eftX:SetVisible( false )
  end
  
  coroutine.yield()
 end
end

function Interaction_Library_Assist( ui )
 local err = ui:FindChildEntity( "error" )
 err:FindChildEntity( "text" ):Set( "NO MATCH RESULT", 2 )
 local cursorX = ui:FindChildEntity( "cursor_x" )
 local cursorY = ui:FindChildEntity( "cursor_y" )
 local cursorPos = { 0, 0 }
 local cursorRegion = { -264, -160, 264, 352 }

 local options
 local nSelectedOption = 0
 local entryStack = {}
 local function DlgEntry( tbl, nOpr )
  if tbl then
   for i = 1, #entryStack, 1 do
    if entryStack[i] == tbl then
	 for j = #entryStack, i + 1, -1 do
	  table.remove( entryStack )
	 end
	 nOpr = -1
	 break
    end
   end
  end
  
  if nOpr ~= -1 then
   if nOpr == 1 then
    entryStack[#entryStack + 1] = tbl
   elseif nOpr == 2 then
    table.remove( entryStack, #entryStack )
    tbl = entryStack[#entryStack]
   else
    entryStack[#entryStack] = tbl
   end
  end

  local contents = tbl.contents
  if type( contents ) == "function" then
   contents = contents()
  end

  local texts = {}
  options = {}
  for i = 1, #contents, 1 do
   local content = contents[i]
   if type( content ) == "function" then
    content = content()
   end
   if i == 1 then
    texts[i] = content
   else
    texts[i] = content[1]
    if type( texts[i] ) == "function" then
     texts[i] = texts[i]()
    end
	options[i - 1] = content[2]
   end
  end

  ui:Refresh( table.unpack( texts ) )
  if #options >= 1 then
   nSelectedOption = 1
   ui:SelectOption( 1 )
  end
 end
 DlgEntry( g_libassist_dialogues[1], 1 )
 
 while not IsKeyDown( VK_ESCAPE ) do
  coroutine.yield()
  
  if IsInput( 0 ) then
   cursorPos[1] = math.min( cursorRegion[3] - 4, cursorPos[1] + 4 )
   SetPosition( cursorX, cursorPos[1], 0 )
  end
  if IsInput( 1 ) then
   cursorPos[2] = math.min( cursorRegion[4] - 4, cursorPos[2] + 4 )
   SetPosition( cursorY, 0, cursorPos[2] )
  end
  if IsInput( 2 ) then
   cursorPos[1] = math.max( cursorRegion[1], cursorPos[1] - 4 )
   SetPosition( cursorX, cursorPos[1], 0 )
  end
  if IsInput( 3 ) then
   cursorPos[2] = math.max( cursorRegion[2], cursorPos[2] - 4 )
   SetPosition( cursorY, 0, cursorPos[2] )
  end

  local strKeyWord = ui:PickWord( cursorPos[1], cursorPos[2] );
  local bContinue = false
  if #strKeyWord and IsInputDown( 4 ) then
   bContinue = true
   local tbl = g_libassist_dialogues[string.lower( strKeyWord )]
   if not tbl then
    PlaySoundEffect( "btn_error" )
    err:SetVisible( true )
	Delay( 40 )
	err:SetVisible( false )
   else
    DlgEntry( tbl, 1 )
   end
  end

  if #options > 0 and not bContinue then
   if IsKeyDown( 5 ) then
    nSelectedOption = nSelectedOption + 1
    if nSelectedOption > #options then
     nSelectedOption = 1
    end
    ui:SelectOption( nSelectedOption )
   end
   if IsKeyDown( 7 ) then
    nSelectedOption = nSelectedOption - 1
    if nSelectedOption <= 0 then
     nSelectedOption = #options
    end
    ui:SelectOption( nSelectedOption )
   end

   if IsKeyDown( VK_RETURN ) or IsKeyDown( string.byte( " " ) ) or IsInputDown( 4 ) then
    local nxt = options[nSelectedOption]
    if type( nxt ) == "function" then
     nxt = nxt()
    end
	if nxt == "~" then
     DlgEntry( tbl, 2 )
	else
	 local nOpr = 0
	 if string.sub( nxt, 1, 1 ) == ":" then
	  nOpr = 1
	  nxt = string.sub( nxt, 2 )
	 end
     local tbl = g_libassist_dialogues[string.lower( nxt )]
     if not tbl then return end
     DlgEntry( tbl, nOpr )
	end
   end
  end
 end

end

function Interaction_Vending_Machine_1( ui )
 local n = EvaluateKeyInt( "$v_knifes" )
 n = n + 1
 SetKeyInt( "$v_knifes", n )
 for i = 1, 3, 1 do
  local p = ui:FindChildEntity( tostring( i ) )
  if i < n then SetImgTexRect( p, { 0.59375, 0.4375 + ( i - 1 ) * 0.03125, 0.03125, 0.03125 } )
  else p:SetVisible( false ) end
 end

 local p = ui:FindChildEntity( tostring( n ) )
 Delay( 20 )
 p:SetVisible( true )
 for i = 1, 3, 1 do
  SetImgTexRect( p, { 0.5 + 0.03125 * i, 0.4375 + ( n - 1 ) * 0.03125, 0.03125, 0.03125 } )
  Delay( 20 )
 end
 Delay( 40 )
 if n == 3 then
  local c = ui:FindChildEntity( "c" )
  for i = 1, 4, 1 do
   SetImgTexRect( c, { 0.375 + 0.03125 * i, 0.5625, 0.03125, 0.0625 } )
   Delay( 20 )
  end
  c:SetVisible( false )
  SetLabelKey( "_COIN_4", 1 )
  Delay( 40 )
 end
end