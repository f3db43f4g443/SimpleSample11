g_labels = {
	{
		day = 1,
		items = {
		}
	},
	{
		day = 2,
		items = {
		}
	},
	{
		day = 3,
		items = {
		}
	},
	{
		day = 4,
		items = {
			{ name = "_DISINFECTANT", key = "%_DISINFECTANT", value = { { 0, 7 } } },
			{ name = "_HANDLE", key = "%_HANDLE", value = { { 1, 7 }, { 7, 7 } } },
			{ name = "_TAPE", key = "%_TAPE", value = { { 2, 7 }, { 2, 7 }, { 2, 7 }, { 7, 7 } } },
			{ name = "_BOOK_1", key = "%_BOOK_1", value = { { 3, 7 }, { 7, 7 } } },
			{ name = "_BOOK_2", key = "%_BOOK_2", value = { { 4, 7 }, { 7, 7 } } },
			{ name = "_BOOK_3", key = "%_BOOK_3", value = { { 5, 7 }, { 7, 7 } } },
		}
	},
}
for k, v in ipairs( g_labels ) do
	local items = v.items
	local itemtbl = {}
	v.itemtbl = itemtbl
	for k1, v1 in ipairs( items ) do
		itemtbl[v1.name] = k1
	end
end


function RefreshLabel( name )
	local labels = g_labels[CurDay()]
	local i = labels.itemtbl[name]
	if i then
		local item = labels.items[i]
		local res = item.value[EvaluateKeyInt( item.key )]
		if res then
			GetMasterLevel():GetMainUI():SetLabel( i - 1, res[1], res[2] )
		else
			GetMasterLevel():GetMainUI():SetLabel( i - 1, -1, -1 )
		end
	end
end

function GetLabelKey( name )
	local labels = g_labels[CurDay()]
	local i = labels.itemtbl[name]
	if i then
		local item = labels.items[i]
		return EvaluateKeyInt( item.key )
	end
	return 0
end

function SetLabelKey( name, value )
	local labels = g_labels[CurDay()]
	local i = labels.itemtbl[name]
	if i then
		local item = labels.items[i]
		SetKeyInt( item.key, value )
		local res = item.value[value]
		if res then
			GetMasterLevel():GetMainUI():SetLabel( i - 1, res[1], res[2] )
		else
			GetMasterLevel():GetMainUI():SetLabel( i - 1, -1, -1 )
		end
	end
end

function RefreshAllLabels()
	GetMasterLevel():GetMainUI():ClearLabels()
	local labels = g_labels[CurDay()].items
	for k, v in ipairs( labels ) do
		local res = v.value[EvaluateKeyInt( v.key )]
		if res then
			GetMasterLevel():GetMainUI():SetLabel( k - 1, res[1], res[2] )
		end
	end
end