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
   if IsChar( keys[i] ) then
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
 local n = EvaluateKeyInt( "%library_window_broken" ) > 0 and 10 or 9
 for i = 1, n, 1 do
  if EvaluateKeyInt( "_CINEMA_SEAT_" .. tostring( i ) ) > 0 then
   local x = EvaluateKeyInt( "_CINEMA_SEAT_X_" .. tostring( i ) )
   local y = EvaluateKeyInt( "_CINEMA_SEAT_Y_" .. tostring( i ) )
   local npc = level:GetPawnByName( tostring( i ) )
   npc:PlayStateForceMove( "", x, y, npc:GetCurDir() )
  end
 end
end

function Reset_Cinema_Seat()
 for i = 1, 10, 1 do
  SetKeyInt( "_CINEMA_SEAT_" .. tostring( i ), 0 )
 end
end

function Interaction_Cinema_Seat( ui, pawn )
 local npcs = {}
 local npcData = { { 6, 4, 5 }, { 6, 2, 1 }, { 9, 3, 1 }, { 10, 0, 1 }, { 14, 2, 2 }, { 8, 8, 2 }, { 4, 6, 2 }, { 15, 5, 3 }, { 10, 2, 4 }, { 13, 5, 3 } }
 local npcGrids = { { { 6, 4 }, { 5, 5 }, { 5, 1 }, { 10, 2 }, { 15, 1 }, { 14, 4 }, { 15, 5 } },
  { { 6, 2 }, { 4, 2 }, { 5, 1 } },
  { { 9, 3 }, { 10, 4 }, { 11, 3 }, { 10, 2 } },
  { { 10, 0 }, { 12, 0 }, { 8, 0 } },
  { { 14, 2 }, { 16, 2 }, { 4, 2 }, { 6, 2 } },
  { { 8, 8 }, { 12, 8 }, { 14, 8 }, { 6, 8 } },
  { { 4, 6 }, { 6, 6 }, { 10, 6 }, { 14, 6 }, { 16, 6 } },
  { { 15, 5 }, { 16, 6 }, { 10, 0 }, { 14, 4 } },
  { { 10, 2 }, { 9, 3 }, { 7, 5 }, { 6, 6 }, { 12, 0 } },
  { { 13, 5 }, { 14, 6 }, { 8, 0 }, { 10, 2 }, { 11, 3 } },
 }
 
 local nNpc = EvaluateKeyInt( "%library_window_broken" ) > 0 and 10 or 9
 for i = 1, nNpc, 1 do
  local data = npcData[i]
  if EvaluateKeyInt( "_CINEMA_SEAT_" .. tostring( i ) ) > 0 then
   data[1] = EvaluateKeyInt( "_CINEMA_SEAT_X_" .. tostring( i ) )
   data[2] = EvaluateKeyInt( "_CINEMA_SEAT_Y_" .. tostring( i ) )
  end
  npcs[i] = ui:FindChildEntity( tostring( i ) )
  SetPosition( npcs[i], data[1] * 24 - 264, data[2] * 32 - 160 )
 end



 if nNpc == 9 then
  ui:FindChildEntity( tostring( 10 ) ):SetVisible( false )
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

   for i = 1, nNpc, 1 do
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
 if EvaluateKeyInt( "%library_assist_login" ) < 0 then
  HeadText( "[Not responding]", htx_color_5, 240 )
  return
 end
 local err = ui:FindChildEntity( "error" )
 err:SetVisible( false )
 err:FindChildEntity( "text" ):Set( "NO MATCH RESULT", 2 )
 local cursorX = ui:FindChildEntity( "cursor_x" )
 local cursorY = ui:FindChildEntity( "cursor_y" )
 local cursorPos = { 0, 0 }
 local cursorRegion = { -320, -256, 320, 320 }
 local pwdui = ui:FindChildEntity( "pwd" )
 pwdui:SetVisible( false )
 local tips = ui:FindChildEntity( "tips" )
 local TIPS_0 = "[I/K, ENTER] SELECT   [WASD, J] PICK"
 local TIPS_PWD = "PLEASE LOGIN - ENTER PASSWORD"
 tips:Set( TIPS_0 )

 local isLogin = EvaluateKeyInt( "%library_assist_login" ) ~= 0
 ui:FindChildEntity( "0/kb/1" ):SetVisible( not isLogin )
 ui:FindChildEntity( "0/kb/2" ):SetVisible( isLogin )
 local menu = ui:FindChildEntity( "menu" )
 menu:SetVisible( false )

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

 ui:PickWord( 10000, 10000 )
 cursorX:SetVisible( false )
 cursorY:SetVisible( false )
 local pwd = EvaluateKeyString( "%library_assist_pwd" )
 local spray1 = ui:FindChildEntity( "spray/1" )
 if pwd == "" then
  SetImgParam( spray1, { 0, 0, 0, 0.99 } )

  if GetLabelKey( "_SPRAY_1" ) == 1 then
   SetLabelKey( "_SPRAY_1", 2 )
  
   for i = 1, 6, 1 do
    local p = spray1:FindChildEntity( tostring( i ) )
    local n = RandInt( 0, 10 )
    SetImgTexRect( p, { ( n % 5 * 2 + 12 ) / 32, ( n // 5 * 2 + 21 ) / 32, 1.0 / 16, 1.0 / 16 } )
    pwd = pwd .. tostring( n )
   end
   SetKeyString( "%library_assist_pwd", pwd )
   PlaySoundEffect( "spray" )
   for i = 1, 15, 1 do
    SetImgParam( spray1, { 0, 0, 0, ( 15 - i ) / 15 } )
	Delay( 10 )
   end
  else
   for i = 1, 6, 1 do
    spray1:FindChildEntity( tostring( i ) ):SetVisible( false )
   end
  end
 else
  for i = 1, 6, 1 do
   local p = spray1:FindChildEntity( tostring( i ) )
   local n = tonumber( string.sub( pwd, i, i ) )
   SetImgTexRect( p, { ( n % 5 * 2 + 12 ) / 32, ( n // 5 * 2 + 21 ) / 32, 1.0 / 16, 1.0 / 16 } )
  end
  SetImgParam( spray1, { 0, 0, 0, 0 } )
 end

 cursorX:SetVisible( true )
 cursorY:SetVisible( true )

 
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
  
  local bContinue = false
  if isLogin then
   if not menu:IsVisible() then
    if cursorPos[1] <= -192 and cursorPos[2] <= -224 and IsInputDown( 4 ) then
	 bContinue = true
     menu:SetVisible( true )
    end
   else
    if cursorPos[1] <= -192 and cursorPos[2] <= -224 and IsInputDown( 4 ) then
	 bContinue = true
     menu:SetVisible( false )
	elseif cursorPos[1] <= -192 and cursorPos[2] > -160 and cursorPos[2] <= -96 and IsInputDown( 4 ) then
	 GetMasterLevel():GotoInteractionUI( "data/interaction/library_assist_1.pf" )
	 return
    end
   end 
  end
  if not isLogin and cursorPos[1] <= -192 and cursorPos[2] <= -224 and IsInputDown( 4 ) then
   pwdui:SetVisible( true )
   cursorX:SetVisible( false )
   cursorY:SetVisible( false )
   tips:Set( TIPS_PWD )

   if Interaction_DoorLock( pwdui, "%library_assist_pwd" ) then
    ui:FindChildEntity( "0/kb/1" ):SetVisible( false )
    ui:FindChildEntity( "0/kb/2" ):SetVisible( true )
	isLogin = true
   end
   coroutine.yield()
   pwdui:SetVisible( false )
   cursorX:SetVisible( true )
   cursorY:SetVisible( true )
   tips:Set( TIPS_0 )
   bContinue = true
  end
  local strKeyWord = ui:PickWord( cursorPos[1], cursorPos[2] )
  if not bContinue and #strKeyWord and IsInputDown( 4 ) then
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
   if IsInputDown( 5 ) then
    nSelectedOption = nSelectedOption + 1
    if nSelectedOption > #options then
     nSelectedOption = 1
    end
    ui:SelectOption( nSelectedOption )
   end
   if IsInputDown( 7 ) then
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

function Interaction_Library_Assist_1( ui )
 local spray1 = ui:FindChildEntity( "spray/1" )
 local pwd = EvaluateKeyString( "%library_assist_pwd" )
 for i = 1, 6, 1 do
  local p = spray1:FindChildEntity( tostring( i ) )
  local n = tonumber( string.sub( pwd, i, i ) )
  SetImgTexRect( p, { ( n % 5 * 2 + 12 ) / 32, ( n // 5 * 2 + 21 ) / 32, 1.0 / 16, 1.0 / 16 } )
 end
 SetImgParam( spray1, { 0, 0, 0, 0 } )

 local back = ui:FindChildEntity( "back" )
 local page1 = ui:FindChildEntity( "1" )
 local page2 = ui:FindChildEntity( "2" )
 page1:SetVisible( false )
 page2:SetVisible( false )
 SetImgParam( back, { 0, 0, 0, 0 } )
 Delay( 80 )
 SetImgParam( back, { 1, 1, 1, 1 } )
 Delay( 15 )
 SetImgParam( back, { 0.8, 0.1, 0.4, 1 } )
 Delay( 10 )
 SetImgParam( back, { 0.2, 0.6, 0.4, 1 } )
 Delay( 5 )
 SetImgParam( back, { 0.02, 0.01, 0.05, 1 } )
 Delay( 60 )
 page1:SetVisible( true )

 local p1 = page1:GetRoot():FindChildEntity( "a/0" )
 local p2 = page1:GetRoot():FindChildEntity( "a/1" )
 local p3 = page1:GetRoot():FindChildEntity( "a/2" )
 p2:SetVisible( false )
 p3:SetVisible( false )
 SetImgParam( p1, { 0.027, 0.11, 0.129, 1 } )

 local h = 576
 local scrollRange = { -512, 512 }
 local yRange = { scrollRange[1] + h * 0.5, scrollRange[2] - h * 0.5 }
 local curY = yRange[2]
 page1:SetCamPos( { 0, curY } )
 page1:GetRoot():FindChildEntity( "a" ):SetVisible( false );
 page1:Refresh()
 Delay( 80 )
 page1:GetRoot():FindChildEntity( "a" ):SetVisible( true );
 page1:Refresh()

 while true do
  coroutine.yield( 1 )
  
  local bRefresh = false
  local y0 = math.floor( curY )

  local bDown = IsInput( 5 ) or IsInput( 3 )
  local bUp = IsInput( 7 ) or IsInput( 1 )
  if bDown then
   if curY > yRange[1] then curY = math.max( yRange[1], curY - 2 )
   else
    curY = curY - ( ( curY - yRange[1] + 128 ) / 128 * 1 + 0.25 )
	if curY < yRange[1] - 128 then break end
	local k = ( curY - yRange[1] + 128 ) / 128
    SetImgParam( p1, { 0.027 * k + 0.16 * ( 1 - k ), 0.11 * k + 0.16 * ( 1 - k ), 0.129 * k + 0.16 * ( 1 - k ), 1 } )
	page1:Refresh()
   end
  elseif bUp then
   if curY >= yRange[1] then curY = math.min( yRange[2], curY + 2 )
   else
    curY = curY + ( yRange[1] - curY ) / 128 * 8 + 4
	local k = ( curY - yRange[1] + 128 ) / 128
    SetImgParam( p1, { 0.027 * k + 0.16 * ( 1 - k ), 0.11 * k + 0.16 * ( 1 - k ), 0.129 * k + 0.16 * ( 1 - k ), 1 } )
	page1:Refresh()
   end
  else
   if curY < yRange[1] then
    curY = curY + ( yRange[1] - curY ) / 128 * 4 + 2
	local k = ( curY - yRange[1] + 128 ) / 128
    SetImgParam( p1, { 0.027 * k + 0.16 * ( 1 - k ), 0.11 * k + 0.16 * ( 1 - k ), 0.129 * k + 0.16 * ( 1 - k ), 1 } )
	page1:Refresh()
   end
  end

  if math.floor( curY ) ~= y0 then
   page1:SetCamPos( { 0, curY } )
   page1:Refresh()
  end
 end

 p1:SetVisible( false )
 p3:SetVisible( true )
 page1:SetCamPos( { 0, yRange[1] - 316 } )
 page1:Refresh()
 Delay( 100 )
 
 SetImgParam( back, { 0.5, 0.4, 0.3, 1 } )
 page1:SetVisible( false )
 Delay( 5 )
 SetImgParam( back, { 0.02, 0.01, 0.05, 1 } )
 page1:SetVisible( true )
 p3:SetVisible( false )
 p2:SetVisible( true )
 page1:SetCamPos( { 0, yRange[1] - 64 } )
 local p2_1 = p2:FindChildEntity( "1" )
 for i = 1, 64, 1 do
  SetImgTexRect( p2_1, { 0.5 + 0.03125 * ( i % 8 ), 0.0625, 0.03125, 0.03125 } )
  page1:Refresh()
  Delay( 6 )
 end
 Delay( 60 )

 page1:SetVisible( false )
 page2:SetVisible( true )
 page2:Refresh()
 local ofsX = 0
 local ofsY = 0

 local k = 0
 while true do
  coroutine.yield( 1 )

  if IsInput( 0 ) then ofsX = ofsX + 2 end
  if IsInput( 1 ) then ofsY = ofsY + 2 end
  if IsInput( 2 ) then ofsX = ofsX - 2 end
  if IsInput( 3 ) then ofsY = ofsY - 2 end
  k = k + 1
  if k == 10 then
   k = 0
   page2:SetClipOfs( { ofsX, -ofsY } )
   page2:Refresh()
   PlaySoundEffect( "btn_error" )
  end
  if ofsX * ofsX + ofsY * ofsY > 512 * 512 then break end
 end
 coroutine.yield( 1 )

 k = 0
 local l = 0
 while true do
  page1:CopyFrom( page2 )
  ofsX = 0
  ofsY = 0
  for i = 1, 60, 1 do
   if IsInput( 0 ) then ofsX = ofsX + 2 l = l + 2 end
   if IsInput( 1 ) then ofsY = ofsY + 2 l = l + 2 end
   if IsInput( 2 ) then ofsX = ofsX - 2 l = l + 2 end
   if IsInput( 3 ) then ofsY = ofsY - 2 l = l + 2 end
   k = k + 1
   if k == 6 then
    k = 0
    page2:SetClipOfs( { ofsX, -ofsY } )
    page2:Refresh()
    PlaySoundEffect( "bzzz0" )
   end
   coroutine.yield( 1 )
  end
  if l > 1000 then break end
 end
 for i = 1, 60, 1 do
  Delay( 2 )
  PlaySoundEffect( "bzzz0" )
 end
 
 SetKeyInt( "%library_assist_login", -1 )
 RunScenario( function()
  GetCurLevel():GetPawnByName( "t" ):SetLocked( true )
  Delay( 60 )
  WaitFor( ScenarioDialogue( 1, "What's that?", dtx_color_6, 120 ), 60 )
  WaitFor( ScenarioDialogue( 1, "Someone just logged in.", dtx_color_6, 120 ), 60 )
  WaitFor( ScenarioDialogue( 1, "It must be " .. NAME_LAWYER .. ".", dtx_color_6, 120 ), 80 )
  Delay( 80 )
  GetMasterLevel():BlackOut( 40, 20 )
  local pawn0 = GetCurLevel():SpawnPawn( 0, 1, 5, 0 )
  local pawn1 = GetCurLevel():SpawnPawn( 0, 17, 5, 1 )
  local pawn2 = GetCurLevel():SpawnPawn( 0, 6, 0, 0 )
  Delay( 80 )
  WaitFor( ScenarioDialogue( 1, "Fuck " .. NAME_LAWYER .. ".", dtx_color_6, 120 ), 60 )
  pawn0:PlayState( "move_x" )
  pawn1:PlayState( "move_x" )
  pawn2:PlayState( "move_up" )
  GetCurLevel():BeginTracer1( 0, 240 )
  Delay( 30 )
 end )
 return
end

function Interaction_Scenario1( ui )
 local n0 = EvaluateKeyInt( "$n0" )
 local n1 = EvaluateKeyInt( "$n1" )

 local a = ui:FindChildEntity( "a" )
 local b = ui:FindChildEntity( "b" )
 local c = ui:FindChildEntity( "c" )
 local d = ui:FindChildEntity( "d" )
 a:SetVisible( false )
 b:SetVisible( false )
 c:SetVisible( false )
 d:SetVisible( false )
 local tbl = {}
 for i = 1, 12, 1 do
  tbl[i] = ui:FindChildEntity( tostring( i - 1 ) )
  tbl[i]:SetVisible( false )
 end
 
 local n = 0
 local function SetFrame( nFrame )
  if n > 0 then tbl[n]:SetVisible( false ) end
  n = nFrame
  if n > 0 then
   tbl[n]:SetVisible( true )
   SetPosition( a, GetX( tbl[n] ), GetY( tbl[n] ) )
   SetPosition( b, GetX( tbl[n] ), GetY( tbl[n] ) )
   SetPosition( c, GetX( tbl[n] ), GetY( tbl[n] ) )
   SetPosition( d, GetX( tbl[n] ), GetY( tbl[n] ) )
  end
 end

 local F1 = function()
  a:SetVisible( true )
  c:SetVisible( true )
  Delay( 2 )
  a:SetVisible( false )
  b:SetVisible( true )
  c:SetVisible( false )
  Delay( 1 )
  c:SetVisible( true )
  Delay( 1 )
  a:SetVisible( true )
  b:SetVisible( false )
  c:SetVisible( false )
 end
 local F2 = function()
  a:SetVisible( false )
  b:SetVisible( true )
  c:SetVisible( false )
  Delay( 6 )
 end
 local F3 = function()
  SetFrame( n1 )
  a:SetVisible( true )
  local t1 = n1 > 3 and 1 or 2
  Delay( 4 * t1 )
  for i = 1, 8, 1 do
   d:SetVisible( true )
   Delay( t1 )
   SetFrame( i > 1 and n1 + 1 or n1 )
   Delay( t1 )
   SetFrame( i > 3 and n1 + 1 or n1 )
   d:SetVisible( false )
   Delay( t1 )
   SetFrame( i > 5 and n1 + 1 or n1 )
   Delay( t1 )
   SetFrame( i > 7 and n1 + 1 or n1 )
  end
  Delay( 4 * t1 )
 end
 local F4 = function()
  for i = n1, 1, -1 do
   SetFrame( i )
   a:SetVisible( false )
   b:SetVisible( true )
   c:SetVisible( false )
   Delay( 5 )
   a:SetVisible( true )
   b:SetVisible( false )
   c:SetVisible( false )
   Delay( 5 )
  end
 end
 local F5 = function()
  for i = 9, 11, 1 do
   if i > 9 then F1() end
   SetFrame( i )
   Delay( 60 )
  end
 end

 if n0 == 0 then
  SetFrame( 1 )
  F1()
  Delay( 25 )
  F2()
 elseif n0 == 1 then
  SetFrame( 1 )
  F1()
  Delay( 15 )
  for i = 2, 5, 1 do
   SetFrame( i )
   Delay( 10 )
  end
  F2()
  for i = 4, 1, -1 do
   SetFrame( i )
   Delay( 2 )
  end
 elseif n0 == 2 then
  F3()
  n1 = n1 + 1
  SetKeyInt( "$n1", n1 )
  if n1 == 9 then F5() end
 elseif n0 == 3 then
  F4()
  SetKeyInt( "$n1", 1 )
 elseif n0 == 4 then
  a:SetVisible( true )
  F5()
 elseif n0 == 5 then
  a:SetVisible( true )
  F5()
  SetFrame( 12 )
  F1()
  Delay( 80 )
  for i = 1, 10, 1 do
   d:SetVisible( true )
   c:SetVisible( true )
   a:SetVisible( false )
   Delay( 3 )
   d:SetVisible( false )
   c:SetVisible( false )
   a:SetVisible( true )
   Delay( 3 )
  end
  c:SetVisible( true )
  for i = 1, 40, 1 do
   d:SetVisible( true )
   Delay( 1 )
   d:SetVisible( false )
   Delay( 1 )
  end
 else
  SetFrame( 12 )
  SetFrame( 0 )
  for i = 1, 10, 1 do
   d:SetVisible( true )
   c:SetVisible( true )
   a:SetVisible( false )
   Delay( 3 )
   d:SetVisible( false )
   c:SetVisible( false )
   a:SetVisible( true )
   Delay( 3 )
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