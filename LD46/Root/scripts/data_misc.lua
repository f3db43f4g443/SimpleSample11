g_videos = {
	{
		strName = "22-08-1971(1)",
		funcScenario = function()
			Delay( 30 )
			GetCurLevel():GetPawnByName( "tv" ):FindChildEntity( "screen" ):SetVisible( true )
			Delay( 60 )
			
			WaitFor( ScenarioDialogue( 0, ".................", dtx_color_6, -1, 6 ) )
			Delay( 60 )
			WaitFor( ScenarioDialogue( 0, "...Why can't I...", dtx_color_6a, -1, 3 ) )
			WaitFor( ScenarioDialogue( 1, "I've told you this form is invalid. Fill another and come again.", dtx_color_6, -1 ) )
			WaitFor( ScenarioDialogue( 0, "...But why...All these are true...I promise...", dtx_color_6a, -1, 3 ) )
			WaitFor( ScenarioDialogue( 1, "...How many times do I need to say to make you understand...", dtx_color_6, -1 ) )
			WaitFor( ScenarioDialogue( 1, "...In COMMON LANGUAGE. No your gross slang.", dtx_color_6, -1 ) )
			WaitFor( ScenarioDialogue( 1, "I don't want to see those illegible marks. Not a single.", dtx_color_6, -1 ) )
			WaitFor( ScenarioDialogue( 1, "Don't take them within your lines full of mistakes. Is that difficult?", dtx_color_6, -1 ) )
			WaitFor( ScenarioDialogue( 0, "...But...this place, how do I...", dtx_color_6a, -1, 3 ) )
			WaitFor( ScenarioDialogue( 1, "What the f...oolish......Am I your language teacher?", dtx_color_6, -1 ) )
			WaitFor( ScenarioDialogue( 0, "...I...I don't know...", dtx_color_6a, -1, 3 ) )
			WaitFor( ScenarioDialogue( 1, "...............", dtx_color_6, -1, 6 ) )
			WaitFor( ScenarioDialogue( 1, "Let me tell you. Go back and get a copy of *NEED*.", dtx_color_6, -1, 3 ), 300 )
			WaitFor( ScenarioDialogue( 1, "It will cover everything.", dtx_color_6, -1, 3 ) )
			WaitFor( ScenarioDialogue( 1, "Please don't waste my time here. I'm really out of patience.", dtx_color_6, -1 ) )
			WaitFor( ScenarioDialogue( 0, "...But I have", dtx_color_6a, 0, 3 ) )
			GetCurLevel():GetPawnByName( "tv" ):FindChildEntity( "screen" ):SetVisible( false )
			Delay( 60 )
			if EvaluateKeyInt( "%video_unlocked_2" ) == 0 then
				Delay( 60 )
				WaitFor( ScenarioDialogue( 0, "Broke off here...There must be the other half...", dtx_color_1, -1 ) )
				if CurDay() == 4 and CurTime() == 1 then
					FEVT( "bookname_unlocked" )
					WaitFor( ScenarioDialogue( 0, "...Forget that. I've already got it.", dtx_color_1, -1 ) )
				end
			end
		end
	},
	{
		strName = "22-08-1971(2)",
		funcScenario = function()
			Delay( 30 )
			GetCurLevel():GetPawnByName( "tv" ):FindChildEntity( "screen" ):SetVisible( true )
			WaitFor( ScenarioDialogue( 0, "no money...", dtx_color_6a, -1, 3 ) )
			WaitFor( ScenarioDialogue( 1, "Then go find a job.", dtx_color_6, -1 ) )
			WaitFor( ScenarioDialogue( 0, "...But I need an ID card...", dtx_color_6a, -1, 3 ) )
			WaitFor( ScenarioDialogue( 1, "Then go apply for it.", dtx_color_6, -1, 2 ) )
			WaitFor( ScenarioDialogue( 0, "...But they said I must have this document...", dtx_color_6a, -1, 3 ) )
			WaitFor( ScenarioDialogue( 1, "Then fill a form properly.", dtx_color_6, -1, 3 ) )
			WaitFor( ScenarioDialogue( 0, "...But...I don't...", dtx_color_6a, -1, 3 ) )
			WaitFor( ScenarioDialogue( 1, "...............", dtx_color_6, -1, 6 ) )
			WaitFor( ScenarioDialogue( 1, "Go back and come tomorrow. The next one is waiting.", dtx_color_6, -1 ) )
			WaitFor( ScenarioDialogue( 0, "...............", dtx_color_6a, -1, 6 ) )
			WaitFor( ScenarioDialogue( 0, "...But...No, please...", dtx_color_6a, -1, 3 ) )
			WaitFor( ScenarioDialogue( 1, "I said come tomorrow. I have no time for you. Get out now.", dtx_color_6, -1 ) )
			WaitFor( ScenarioDialogue( 0, "...Please...I really need this...", dtx_color_6a, -1, 3 ) )
			WaitFor( ScenarioDialogue( 1, "GET OUT.", dtx_color_6, -1, 6 ) )
			WaitFor( ScenarioDialogue( 0, "...Please...I...I'm begging...", dtx_color_6a, -1, 3 ) )
			WaitFor( ScenarioDialogue( 1, "GET OUT.", dtx_color_6, -1, 6 ) )
			WaitFor( ScenarioDialogue( 1, "I said GET OUT.", dtx_color_6, -1, 6 ) )
			WaitFor( ScenarioDialogue( 0, "...................", dtx_color_6a, -1, 6 ) )
			WaitFor( ScenarioDialogue( 0, "...Fuck. Fuck it.", dtx_color_6a, -1, 3 ) )
			WaitFor( ScenarioDialogue( 0, "Fuck you. Fuck this. Fuck the form.", dtx_color_6a, -1, 2 ) )
			WaitFor( ScenarioDialogue( 1, "...............", dtx_color_6, -1, 6 ) )
			WaitFor( ScenarioDialogue( 1, "Whatever you say...This is the necessary procedure.", dtx_color_6, -1, 2 ) )
			WaitFor( ScenarioDialogue( 0, "Fuck the procedure. Fuck the ID card. Fuck the job.", dtx_color_6a, -1 ) )
			WaitFor( ScenarioDialogue( 1, "Such nonsences are useless. Please just go out.", dtx_color_6, -1 ) )
			WaitFor( ScenarioDialogue( 0, "Fuck the job. I will never have a fucking job.", dtx_color_6a, -1 ) )
			WaitFor( ScenarioDialogue( 0, "I will never need a fucking job.", dtx_color_6a, -1 ) )
			WaitFor( ScenarioDialogue( 0, "I will never do a fucking job.", dtx_color_6a, -1 ) )
			WaitFor( ScenarioDialogue( 0, "I will go robbing. Go housebreaking. Go looting the shop.", dtx_color_6a, -1 ) )
			WaitFor( ScenarioDialogue( 1, "...Then get out and do it. Now. Join those mobs outside.", dtx_color_6, -1 ) )
			WaitFor( ScenarioDialogue( 0, "Go heisting the biggest bank. And get shot by a fucking cop.", dtx_color_6a, -1 ) )
			WaitFor( ScenarioDialogue( 0, "Rather than doing a motherfucking job.", dtx_color_6a, -1 ) )
			WaitFor( ScenarioDialogue( 1, "Get out of my sight. Go acting mad somewhere else.", dtx_color_6, -1 ) )
			WaitFor( ScenarioDialogue( 0, "Fuck you. Go hell with the job. I'll do it. I'm doing it now.", dtx_color_6a, -1 ) )
			WaitFor( ScenarioDialogue( 0, "...............", dtx_color_6a, -1 ) )
			WaitFor( ScenarioDialogue( 1, "...............", dtx_color_6, -1, 6 ) )
			WaitFor( ScenarioDialogue( 1, "...What a dick.", dtx_color_6, -1, 3 ) )
			Delay( 30 )
			GetCurLevel():GetPawnByName( "tv" ):FindChildEntity( "screen" ):SetVisible( false )
			Delay( 60 )

			if CurDay() == 4 and CurTime() == 2 and GetMasterLevel():GetCurLevelName() == "stages/4f_center_room_2a.pf" and FEVT( "$sc_v" ) then
				GetCurLevel():GetPawnByName( "video" ):FindChildEntity( "mount1" ):SetEnabled( false )
				GetCurLevel():GetPawnByName( "video" ):FindChildEntity( "mount2" ):SetEnabled( false )
				WaitFor( ScenarioDialogue( 0, "........", dtx_color_1, -1, 6 ) )
				WaitFor( ScenarioDialogue( 0, "Fuck...Someone is coming...", dtx_color_1, -1, 6 ) )
				LevelRegisterUpdate1( function()
					Delay( 80 )
					GetMasterLevel():BlackOut( 30, 0 )
					local pawn = GetCurLevel():SpawnPawn( 0, 14, 0, 1 )
					pawn:SetEntityName( "a" )
					pawn:GetAI():RunCustomScript( Day4_2_1_AISpecialFunc )
					Delay( 5 )
					SetKeyInt( "4f_center_room_2a_d1", 1 )
				end )
			end
		end
	},
	{
		strName = "****************",
		funcScenario = function() end
	},
	{
		strName = "****************",
		funcScenario = function() end
	},
	{
		strName = "****************",
		funcScenario = function() end
	}
}

g_books = {
	{
		key = "NEED",
		func = function( ui )
			if IsDocUnlocked( "_NEED" ) == 0 then
				if EvaluateKeyInt( "bookname_unlocked" ) == 0 then
					HeadText( "You fucking cheater.", htx_color_h )
					GetCurLevel():Fail( 1 )
					return
				end
				local pFound = ui:FindChildEntity( "found" );
				pFound:SetVisible( true );
				pFound:FindChildEntity( "text" ):Set( "BOOK FOUND\nLOCATION SHOWN\n ON MAP", 2 );
				Delay( 180 )
				GetMasterLevel():UnlockRegionMap( "4f" )
				GetMasterLevel():AddLevelMark( "mark_1", "stages/4f_library_4b.pf", 8, 4 )
				GetMasterLevel():ShowWorldMap( true, 0 )
			end
		end
	},
	{
		key = "EEHA",
		func = function( ui )
			if GetLabelKey( "_BOOK_3" ) == 0 then
				if EvaluateKeyInt( "_BOOKSHELF_TAPE_3" ) == 0 then
					HeadText( "You fucking cheater.", htx_color_h )
					GetCurLevel():Fail( 1 )
					return
				end
				local pFound = ui:FindChildEntity( "found" );
				pFound:SetVisible( true );
				pFound:FindChildEntity( "text" ):Set( "BOOK FOUND\nLOCATION SHOWN\n ON MAP", 2 );
				Delay( 180 )
				GetMasterLevel():UnlockRegionMap( "4f" )
				GetMasterLevel():AddLevelMark( "mark_2", "stages/4f_library_3b.pf", 2, 4 )
				GetMasterLevel():ShowWorldMap( true, 0 )
			end
		end
	},
}

g_libassist_dialogues = {
	{
		strName = "__0",
		contents = {
			[====[
Welcome to the library.

This is the library smart assist system. I am ready to help you with any trouble accessing our services here.

If you have anything to ask, please select one[I/K, ENTER] that matches your question best from the following options. 

If you can't find your question. you can also pick a keyword with the cursor[WASD, J] to search for information.
			]====],
			{ "I have problems accessing library service...", ":__1" },
			{ "I want to know...", ":__2" },
			{ "My problems are solved. Thanks.", "" }
		},
	},
	{
		strName = "__1",
		contents = {
			[====[
What problems did you meet?


			]====],
			{ "I have problems about books...", ":book" },
			{ "I have problems about videos...", ":video" },
			{ "I have some other problems...", "_1_1" },
			{ "I have something else to ask...", "~" }
		},
	},
	{
		strName = "book",
		tblAlias = { "books" },
		contents = {
			[====[
We have a huge storage of books of all categories here, and we update them constantly.

Books of technology, natural science, politics, law, business, and academic papers are stored on 5f.
Social science, humanities, philosophiy, literature, art, documents and other books are on 4f.

All the books here are available to anyone. Before you read books you need to borrow them first. For public health please ]====] .. [====[
don't touch books before you borrow them.
			]====],
			{ "I want to borrow books.", ":_borrow_books" },
			{ "I want to return books.", ":_return_books" },
			{ "I have something else to ask...", "~" },
		},
	},
	{
		strName = "_borrow_books",
		contents = {
			[====[
To borrow books, you need to use the terminal in 4f center room. Input the code of the book you want, and you are granted permission ]====] .. [====[
to access the book. Please remember to return the books in time.
			]====],
			{ "I want to return books.", "_return_books" },
			{ "I have something else to ask...", "~" }
		},
	},
	{
		strName = "_return_books",
		contents = {
			[====[
To return books, just put the book back to its original place. Our library staff collect all books and sort them up regularly, ]====] .. [====[
however it takes lots of time, so please make sure you put them to their own position.
			]====],
			{ "I have something else to ask...", "~" }
		},
	},
	{
		strName = "code",
		contents = {
			[====[
The code of a book is printed on the paper attached to the book spine. Books on bookshelves are sorted by the code. You use the code ]====] .. [====[
instead of the full name of the book when using our library system.
			]====],
			{ "I have something else to ask...", "~" }
		},
	},
	{
		strName = "health",
		tblAlias = { "touch" },
		contents = {
			[====[
[Request failed: Access denied]


			]====],
			{ "[SHOW DETAIL AND REPORT THIS ERROR]", "__error" }
		},
	},
	{
		strName = "video",
		tblAlias = { "videos" },
		contents = {
			[====[
Our library provides high quality video resources. If you want to watch videos, go to the cinema at the center of 3f.

			]====],
			{ "What videos do you have in library?", ":_videos_in_library" },
			{ "How can I borrow videos and take them away?", ":_borrow_videos" },
			{ "I have something else to ask...", "~" },
		},
	},
	{
		strName = "_videos_in_library",
		contents = {
			[====[
For best watching experience, our cinema uses the newest streaming technology and the most advance AI recommendation algorithm. ]====] .. [====[
With these technology we can get the most popular videos all over the world without storing them here.

			]====],
			{ "How can I choose the videos to play?", "_choose_videos" },
			{ "How can I borrow videos and take them away?", "_borrow_videos" },
			{ "I have something else to ask...", "~" },
		},
	},
	{
		strName = "_choose_videos",
		contents = {
			[====[
You don't need to do that on your own. We pick the best videos for you, and what you need to do is to sit there and enjoy it.

			]====],
			{ "How can I borrow videos and take them away?", "_borrow_videos" },
			{ "I have something else to ask...", "~" },
		},
	},
	{
		strName = "_borrow_videos",
		contents = {
			[====[
Because we don't store videos in the library, you can't borrow them like books. However, if you like any videos we've played ]====] .. [====[
ant want to watch it again sometime later, you can put it into the Collection.

			]====],
			{ "How to put videos into the Collection?", "collection" },
			{ "I have something else to ask...", "~" },
		},
	},
	{
		strName = "collection",
		contents = {
			[====[
If you want to put a video into the Collection, please contact our staff. Our staff will record the video and store it inside a Blackbox. ]====] .. [====[
We will keep the recorded videos for you. If you want to watch it again, just tell our staff and we will play it for you.

			]====],
			{ "I have something else to ask...", "~" },
		},
	},
	{
		strName = "blackbox",
		tblAlias = { "blackboxex" },
		contents = {
			[====[
A Blackbox is a portable terminal with high level of data security. Blackboxes are kept by our staff. If you find them in the library, please ]====] .. [====[
contact our staff and don't try to take it out. Our security system will prevent them from being taken away by anyone except our staff.

			]====],
			{ "I have something else to ask...", "~" },
		},
	},

	{
		strName = "__2",
		contents = {
			[====[
What information do you want?


			]====],
			{ "What's this place?", ":here" },
			{ "Who are you?", ":I" },
			{ "What help can get here?", ":help" },
			{ "I have something else to ask...", "~" }
		},
	},
	{
		strName = "here",
		contents = {
			[====[
This is the central library of the health caring center. We are working to provide the best reading experience. Books, videos 
and other materials of all kinds and categories are accessible here.
			]====],
			{ "I have something else to ask...", "~" },
		},
	},
	{
		strName = "i",
		tblAlias = { "we" },
		contents = {
			[====[
I am the library smart assist system. This system is based on the most cutting-edge AI technology. We're here to help you for any problems you met.
			]====],
			{ "What can *AI* technology do?", "ai" },
			{ "I have something else to ask...", "~" }
		},
	},
	{
		strName = "ai",
		tblAlias = { "smart" },
		contents = {
			[====[
AI technology is providing essential support for this system. Put simply, it can make a machine SMART. With AI a ]====] .. [====[
machine can think, learn, feel, and understand, just like a human.
			]====],
			{ "How can AI know what help do I need?", "help" },
			{ "I have something else to ask...", "~" }
		},
	},
	{
		strName = "help",
		contents = {
			[====[
Thanks to the AI technology, we are able to have conversations with people. From the data we gathered ]====] .. [====[
from the information flow and our communications we learn your thoughts and demands. Then we figure out the problems ]====] .. [====[
you possibly have, list them and provide the answer. We try our best to cover all the possible questions so that you ]====] .. [====[
can always get helped.
			]====],
			{ "But I still have some questions that I can't find...", "keyword" },
			{ "I have something else to ask...", "~" }
		},
	},
	{
		strName = "data",
		tblAlias = { "collect", "collected", "gather", "gathered" },
		contents = {
			[====[
We collect user data from multiple sources. This is a very complex mechanic, but we'll comply with the UserAgreement you've accepted.
			]====],
			{ "I have something else to ask...", "~" }
		},
	},
	{
		strName = "useragreement",
		contents = {
			[====[
You have already accepted the UserAgreement. You don't need to confirm it manually, because by using our system you automatically did it.
			]====],
			{ "Where can I read the UserAgreement?", "__read_user_agreement" },
			{ "I have something else to ask...", "~" }
		},
	},
	{
		strName = "__read_user_agreement",
		contents = {
			[====[
[Error: Database query exception: sub statement returned NULL]


			]====],
			{ "[SHOW DETAIL AND REPORT THIS ERROR]", "__error" }
		},
	},
	{
		strName = "keyword",
		contents = {
			[====[
Sometimes our AI analysis fails to meet your demand because the actual scene is too complex. That's why we have the function to pick a keyword. ]====] .. [====[
This feature allows users to search for information they want.
			]====],
			{ "Why can't I input a keyword instead of picking one?", "__input_keyword" },
			{ "I have something else to ask...", "~" }
		},
	},
	{
		strName = "__input_keyword",
		contents = {
			[====[
This feature is currently not available for many reasons. We will support this feature once all requirements are met.
			]====],
			{ "I have something else to ask...", "~" }
		},
	},
	{
		strName = "requirements",
		contents = {
			[====[
[This message has been blocked due to security issues]


			]====],
			{ "[SHOW DETAIL AND REPORT THIS ERROR]", "__error" }
		},
	},
	{
		strName = "__error",
		contents = {
			[====[
^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31
^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31
^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31
^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31
^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31
^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31
^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31
^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31
^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31
^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31
^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31
^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31
^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31
^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31
			]====],
			{ "I have something else to ask...", "~" }
		},
	},
}
for i = 1, #g_libassist_dialogues, 1 do
	local tbl = g_libassist_dialogues[i]
	tbl.nIndex = i
	g_libassist_dialogues[tbl.strName] = tbl
	if tbl.tblAlias then
		for j = 1, #tbl.tblAlias, 1 do
			g_libassist_dialogues[tbl.tblAlias[j]] = tbl
		end
	end
end