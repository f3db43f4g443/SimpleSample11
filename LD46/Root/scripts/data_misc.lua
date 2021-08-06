g_videos = {
	{
		strName = "^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31^31",
		strDest = "stages/unknown_d4_video_secret_room.pf",
		nDestX = 8,
		nDestY = 2,
		nDestDir = 0,
	},
	{
		strName = function()
			return EvaluateKeyInt( "%video_unlocked_2_1" ) > 0 and "17-08-1971(5)" or "[DAMAGED]17-08-1971(5)"
		end,
		strDest = "stages/unknown_c_unlock_4_10.pf",
		nDestX = 3,
		nDestY = 3,
		nDestDir = 0,
	},
	{
		strName = "****************",
		strDest = "stages/unknown_d4_video_secret_room.pf",
		nDestX = 8,
		nDestY = 2,
		nDestDir = 0,
	},
	{
		strName = "****************",
		strDest = "stages/unknown_d4_video_secret_room.pf",
		nDestX = 8,
		nDestY = 2,
		nDestDir = 0,
	},
	{
		strName = "****************",
		strDest = "stages/unknown_d4_video_secret_room.pf",
		nDestX = 8,
		nDestY = 2,
		nDestDir = 0,
	},
}
g_videos["2_1"] = 2

g_books = {
	{
		key = "NEED",
		func = function( ui )
			if not IsDocUnlocked( "_NEED" ) then
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

function Day3_Sampling( n )
	SetKeyInt( "%doc_unlocked_D3_ES_" .. tostring( n ), 1 )
	UnlockDoc( "_D3_ES", true )
	if GetLabelKey( "_SAMPLE" ) >= 2 then return end
	SetLabelKey( "_SAMPLE", 1 )
	local n = GetLabelCounter( "_SAMPLE" )
	n = n + 1
	if n >= 20 then
		SetLabelKey( "_SAMPLE", 2 )
		SetLabelCounter( "_SAMPLE", 0 )
	else
		SetLabelCounter( "_SAMPLE", n )
	end
end