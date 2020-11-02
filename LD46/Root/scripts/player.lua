function InitPlayerData()
 local a = RandInt( 0, 16 )
 local b = RandInt( 0, 15 )
 if b >= a then
  b = b + 1
 end
 local myName = "^" .. tostring( a // 10 ) .. tostring( a % 10 ) .. "^" .. tostring( b // 10 ) .. tostring( b % 10 )
 SetKeyInt( "%myname_0", a )
 SetKeyInt( "%myname_1", b )
 SetKeyString( "%myname_str", myName )

 local str = {"ca", "co", "ti", "te", "to", "si", "se", "su", "ha", "me", "mo", "mu", "ne", "ni", "fa", "do", "la", "le", "lo", "lu",
    "go", "gu", "ba", "bo", "pe", "pu", "ze", "zu" }
 local l = #str
 for i = 1, 16, 1 do
  local n = RandInt( i, l + 1 )
  local tmp = str[i]
  str[i] = str[n]
  str[n] = tmp
  SetKeyString( "%letter_table_" .. tostring( i - 1 ), str[i] )
 end

 local myName1 = str[a + 1] .. str[b + 1]
 if myName1:sub( 3, 3 ) == "l" then
  myName1 = myName1:sub( 1, 2 ) .. "r" .. myName1:sub( 4, 4 )
 end
 if myName1:sub( 4, 4 ) == "e" then
  myName1 = myName1:sub( 1, 2 ) .. myName1:sub( 4, 4 ) .. myName1:sub( 3, 3 )
 end
 local n = string.byte( "A" ) - string.byte( "a" )
 myName1 = string.char( myName1:byte( 1, 1 ) + n, myName1:byte( 2, 2 ) + n, myName1:byte( 3, 3 ) + n, myName1:byte( 4, 4 ) + n )
 SetKeyString( "%myname_str_1", myName1 )
end