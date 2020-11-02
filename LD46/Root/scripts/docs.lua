g_docs = {
	{
		key = "_D2_NOTE",
		name = "A page from a notebook",
		content = [====[
How to win a fight

0 ) Computers are nuts
1 ) Start a fight
2 ) Get beaten
3 ) Fall
4 ) Don't move
5 ) He is a violence user
6 ) He gets punished
7 ) Win
]====]
	},
	{
		key = "_D2_MSG1",
		name = "A reply message [1]",
		content = [====[
......Yes, that's right. You don't need to ask those questions you have the answer. If you forget something, the documentations ]====] .. [====[
are there, and my answer will be no different. I'm here for any help, but please don't bother me too much for those small things, ]====] .. [====[
as I have my own works. It's OK to just submit the daily report.

About that command, if you want to know more, it's some EM wave of certain frequency patterns...No, that's too complicated. I can ]====] .. [====[
only tell it's essentially a defect of human brains. It's just...there's always something they are vulnerable to, something else even ]====] .. [====[
if not this, like they can be smashed by a hammer...You don't need to understand that, just follow the documentation and keep in ]====] .. [====[
mind that you should not use it too much.......]====]
	},
	{
		key = "_D2_MSG2",
		name = "A reply message [2]",
		content = [====[
......I really don't understand why you are always asking me...I've left everything to you, and you certainly know what to ]====] .. [====[
do. It's you who now have the power. I thought there's nothing I need to help with. If you think it's necessary, then just use ]====] .. [====[
it. You are the one responsible for those things, Not me. 

Lastly maybe I should remind you again...the last time. It ultimately depends on you, but I must warn you that though it seems ]====] .. [====[
working, most of its mechanics still remain unclear. It is just an reserved tool. Don't use it as a conventional mean, or I can't ]====] .. [====[
guarantee it will never go wrong......]====]
	},
	{
		key = "_D2_MSG3",
		name = "A reply message [3]",
		content = [====[
......You are blaming me for this...That's not reasonable at all. Yes, I invented this, I authorized you to use it, and I taught ]====] .. [====[
you how to use it, but I didn't force you to do that. I've told you the risk, and it all depends on you, but you got relying on ]====] .. [====[
it, as well as relying on me. You have power on all these, you are responsible for all these, and you screwed it up. Now you come ]====] .. [====[
here and claim it's all my fault, and ask me to pick up the bits...Yes, I can fix this for sure. But you should be aware of the ]====] .. [====[
price...For your safety don't come here tomorrow. Stay in your own room and I'll isolate him and check his condition. And you... ]====] .. [====[
we'll talk on it.]====]
	},
	{
		key = "_D2_DATA_CHUNK",
		name = "Some raw data chunk",
		content = [====[
[data chunk, hardly any readable text]
......
****|P1_****^FLE************|P2_****^DWN****#LNK:P0_********
************|P3_****^DWN****#LNK:P0_************|P4_****^FLE
********************|P0_****^MAD************#KEY{ERR********
********************************************|P5_****^FLE****
********|P6_****^FLE************|P7_****^DWN****#LNK:P0_****
************************************************************
......
**********************************F.U.C.K. .Y.O.U.R. .S.Y.S.
T.E.M.****************T.H.R.O.W. .H.I.M. .O.U.T.************
......]====]
	},

	{
		key = "_NEED",
		name = "National Elementary Education Documents, Page 251",
		content = function()
			local str = "[handwriting]  " .. EvaluateKeyString( "%myname_str" ) .. "\n" .. [====[

...is used for all formal occasions and documents. Our government have finished the conversion of ]====] .. [====[
old records for us. If you've never had a name in Common Language, your new name is converted from the ]====] .. [====[
old name following the steps below:
 1. Replace the old characters with the corresponding Common Language letters. Here is the table:
]====]
			for i = 0, 15, 1 do
				str = str .. "^" .. tostring( i // 10 ) .. tostring( i % 10 ) .. " -> " .. EvaluateKeyString( "%letter_table_" .. tostring( i ) )
				if ( i + 1 ) % 4 == 0 then
					str = str .. "\n"
				else
					str = str .. "   "
				end
			end
			str = str .. [====[
 2. Replace the *l* with *r* in the last syllable. e.g. Lola -> Lora
 3. If the word ends with *e*, swap the last 2 letters. e.g. Hate -> Haet]====]
			return str
		end,
	},
	{
		key = "_LAWYER_NOTE",
		name = "Note from a lawyer",
		content = [====[
Dear client.

Sorry for having no time to meet you. I know I can trust you, but as you know, the evidence is quite unfavorable. You saw the videos ]====] .. [====[
yesterday. They certainly also have it. It clearly proved that you were just nearby at that time undoubtly, as well as your motive.

If you insist that you only witnessed it and just went home then...I think we need more to prove that, but it seems very difficult. I'm ]====] .. [====[
sorry to say that these are all I can help. I'm just a free lawyer and don't have much time...Too many people are waiting for me.

I think the best strategy is to just admit your guilt. It's not that terrible crime as you thought. I'm afraid you regarded that as too ]====] .. [====[
serious. Simply admitting it will let us avoid many unwanted complications. Honestly, they are so buzy that they just want to get it ]====] .. [====[
done as soon as possible...I've heard they are not even going to have a court. 

Please trust me.

Your sincere lawyer]====]
	},
}
for i = 1, #g_docs, 1 do
	local tbl = g_docs[i]
	tbl.nIndex = i
	g_docs[tbl.key] = tbl
end

function GetKeyUnlockDoc( strName )
	return "%doc_unlocked" .. strName
end

function IsDocUnlocked( strName )
	return GetKeyUnlockDoc() > 0
end

function UnlockDoc( strName )
	if FEVT( GetKeyUnlockDoc( strName ) ) then
		local item = g_docs[strName]
		GetMasterLevel():ShowDoc( item.nIndex - 1 )
	end
end