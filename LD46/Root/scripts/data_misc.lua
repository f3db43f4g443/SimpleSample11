g_videos = {
	{
		strName = "22-08-1971(1)",
		funcScenario = function()
			Delay( 30 )
			GetCurLevel():GetPawnByName( "tv" ):FindChildEntity( "screen" ):SetVisible( true )
			Delay( 60 )
			
			WaitFor( ScenarioDialogue( 0, ".................", dtx_color_6, -1, 6 ) )
			Delay( 60 )
			WaitFor( ScenarioDialogue( 0, "...Why can't I...", dtx_color_6, -1, 3 ) )
			WaitFor( ScenarioDialogue( 1, "I've told you this form is invalid. Fill another and come again.", dtx_color_6, -1 ) )
			WaitFor( ScenarioDialogue( 0, "...But why...All these are true...I promise...", dtx_color_6, -1, 3 ) )
			WaitFor( ScenarioDialogue( 1, "...How many times do I need to say to make you understand...", dtx_color_6, -1 ) )
			WaitFor( ScenarioDialogue( 1, "...In COMMON LANGUAGE. No your gross slang.", dtx_color_6, -1 ) )
			WaitFor( ScenarioDialogue( 1, "I don't want to see those illegible marks. Not a single.", dtx_color_6, -1 ) )
			WaitFor( ScenarioDialogue( 1, "Don't take them within your lines full of mistakes. Is that difficult?", dtx_color_6, -1 ) )
			WaitFor( ScenarioDialogue( 0, "...But...this place, how do I...", dtx_color_6, -1, 3 ) )
			WaitFor( ScenarioDialogue( 1, "What the f...oolish......Am I your language teacher?", dtx_color_6, -1 ) )
			WaitFor( ScenarioDialogue( 0, "...I...I don't know...", dtx_color_6, -1, 3 ) )
			WaitFor( ScenarioDialogue( 1, "...............", dtx_color_6, -1, 6 ) )
			WaitFor( ScenarioDialogue( 1, "Let me tell you. Go back and get a copy of *NEED*.", dtx_color_6, -1, 3 ), 300 )
			WaitFor( ScenarioDialogue( 1, "It will cover everything.", dtx_color_6, -1, 3 ) )
			WaitFor( ScenarioDialogue( 1, "Please don't waste my time here. I'm really out of patience.", dtx_color_6, -1 ) )
			WaitFor( ScenarioDialogue( 0, "...But I have", dtx_color_6, 0, 3 ) )
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
			WaitFor( ScenarioDialogue( 0, "no money...", dtx_color_6, -1, 3 ) )
			WaitFor( ScenarioDialogue( 1, "Then go find a job.", dtx_color_6, -1 ) )
			WaitFor( ScenarioDialogue( 0, "...But I need an ID card...", dtx_color_6, -1, 3 ) )
			WaitFor( ScenarioDialogue( 1, "Then go apply for it.", dtx_color_6, -1, 2 ) )
			WaitFor( ScenarioDialogue( 0, "...But they said I must have this document...", dtx_color_6, -1, 3 ) )
			WaitFor( ScenarioDialogue( 1, "Then fill a form properly.", dtx_color_6, -1, 3 ) )
			WaitFor( ScenarioDialogue( 0, "...But...I don't...", dtx_color_6, -1, 3 ) )
			WaitFor( ScenarioDialogue( 1, "...............", dtx_color_6, -1, 6 ) )
			WaitFor( ScenarioDialogue( 1, "Go back and come tomorrow. The next one is waiting.", dtx_color_6, -1 ) )
			WaitFor( ScenarioDialogue( 0, "...............", dtx_color_6, -1, 6 ) )
			WaitFor( ScenarioDialogue( 0, "...But...No, please...", dtx_color_6, -1, 3 ) )
			WaitFor( ScenarioDialogue( 1, "I said come tomorrow. I have no time for you. Get out now.", dtx_color_6, -1 ) )
			WaitFor( ScenarioDialogue( 0, "...Please...I really need this...", dtx_color_6, -1, 3 ) )
			WaitFor( ScenarioDialogue( 1, "GET OUT.", dtx_color_6, -1, 6 ) )
			WaitFor( ScenarioDialogue( 0, "...Please...I...I'm begging...", dtx_color_6, -1, 3 ) )
			WaitFor( ScenarioDialogue( 1, "GET OUT.", dtx_color_6, -1, 6 ) )
			WaitFor( ScenarioDialogue( 1, "I said GET OUT.", dtx_color_6, -1, 6 ) )
			WaitFor( ScenarioDialogue( 0, "...................", dtx_color_6, -1, 6 ) )
			WaitFor( ScenarioDialogue( 0, "...Fuck. Fuck it.", dtx_color_6, -1, 3 ) )
			WaitFor( ScenarioDialogue( 0, "Fuck you. Fuck this. Fuck the form.", dtx_color_6, -1, 2 ) )
			WaitFor( ScenarioDialogue( 1, "...............", dtx_color_6, -1, 6 ) )
			WaitFor( ScenarioDialogue( 1, "Whatever you say...This is the necessary procedure.", dtx_color_6, -1, 2 ) )
			WaitFor( ScenarioDialogue( 0, "Fuck the procedure. Fuck the ID card. Fuck the job.", dtx_color_6, -1 ) )
			WaitFor( ScenarioDialogue( 1, "Such nonsences are useless. Please just go out.", dtx_color_6, -1 ) )
			WaitFor( ScenarioDialogue( 0, "Fuck the job. I will never have a fucking job.", dtx_color_6, -1 ) )
			WaitFor( ScenarioDialogue( 0, "I will never need a fucking job.", dtx_color_6, -1 ) )
			WaitFor( ScenarioDialogue( 0, "I will never do a fucking job.", dtx_color_6, -1 ) )
			WaitFor( ScenarioDialogue( 0, "I will go robbing. Go housebreaking. Go looting the shop.", dtx_color_6, -1 ) )
			WaitFor( ScenarioDialogue( 1, "...Then get out and do it. Now. Join those mobs.", dtx_color_6, -1 ) )
			WaitFor( ScenarioDialogue( 0, "Go heisting the biggest bank. And get shot by a fucking cop.", dtx_color_6, -1 ) )
			WaitFor( ScenarioDialogue( 0, "Rather than doing a motherfucking job.", dtx_color_6, -1 ) )
			WaitFor( ScenarioDialogue( 1, "Get out of my sight. Go acting mad somewhere else.", dtx_color_6, -1 ) )
			WaitFor( ScenarioDialogue( 0, "Fuck you. Go hell with the job. I'll do it. I'm doing it now.", dtx_color_6, -1 ) )
			WaitFor( ScenarioDialogue( 0, "...............", dtx_color_6, -1 ) )
			WaitFor( ScenarioDialogue( 1, "...............", dtx_color_6, -1, 6 ) )
			WaitFor( ScenarioDialogue( 1, "...What a dick.", dtx_color_6, -1, 3 ) )
			Delay( 30 )
			GetCurLevel():GetPawnByName( "tv" ):FindChildEntity( "screen" ):SetVisible( false )
			Delay( 60 )
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