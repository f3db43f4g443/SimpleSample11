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