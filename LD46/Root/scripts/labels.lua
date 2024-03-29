g_labels = {
	{
		day = 1,
		items = {
			{ name = "_DAMAGED", key = "%_DAMAGED", value = { [0] = { 3, 6 }, { 4, 6 } } },
			{ name = "_BUG_1", key = "%_BUG_1", value = { { 7, 6 }, { 7, 7 } } },
			{ name = "_BUG_2", key = "%_BUG_2", value = { { 7, 6 }, { 7, 7 } } },
			{ name = "_BUG_3", key = "%_BUG_3", value = { { 7, 6 }, { 7, 7 } } },
		}
	},
	{
		day = 2,
		items = {
			{ name = "_DAMAGED", key = "%_DAMAGED", value = { [0] = { 3, 6 }, { 4, 6 } } },
		}
	},
	{
		day = 3,
		items = {
			{ name = "_DAMAGED", key = "%_DAMAGED", value = { [0] = { 3, 6 }, { 4, 6 } } },
			{ name = "_SAMPLE", key = "%_SAMPLE", counter = "%_SAMPLE_COUNTER", value = { { 2, 6 }, { 6, 7 }, { 7, 7 } } },
			{ name = "_ELEVATOR_CARD", key = "%_ELEVATOR_CARD", value = { { 5, 6 }, { 7, 7 } } },
			{ name = "_DISINFECTANT", key = "%_DISINFECTANT", value = { { 6, 7 }, { 7, 7 } } },
			{ name = "_COIN_1", key = "%_COIN_1", counter = "%_COIN_1_COUNTER", value = { { 0, 6 }, { 7, 7 } } },
			{ name = "_COIN_2", key = "%_COIN_2", counter = "%_COIN_2_COUNTER", value = { { 0, 6 }, { 7, 7 } } },
			{ name = "_COIN_3", key = "%_COIN_3", counter = "%_COIN_3_COUNTER", value = { { 0, 6 }, { 7, 7 } } },
			{ name = "_COIN_4", key = "%_COIN_4", counter = "%_COIN_4_COUNTER", value = { { 0, 6 }, { 7, 7 } } },
			{ name = "_MONEY", key = "%_MONEY", value = { { 6, 6 }, { 7, 7 } } },
		}
	},
	{
		day = 4,
		items = {
			{ name = "_DAMAGED", key = "%_DAMAGED", value = { [0] = { 3, 6 }, { 4, 6 } } },
			{ name = "_DISINFECTANT", key = "%_DISINFECTANT_DAY4", value = { { 0, 7 }, { 6, 7 } } },
			{ name = "_HANDLE", key = "%_HANDLE", value = { { 1, 7 }, { 7, 7 } } },
			{ name = "_TAPE", key = "%_TAPE", value = { { 2, 7 }, { 2, 7 }, { 2, 7 }, { 7, 7 } } },
			{ name = "_BOOK_1", key = "%_BOOK_1", value = { { 3, 7 }, { 7, 7 } } },
			{ name = "_BOOK_2", key = "%_BOOK_2", value = { { 4, 7 }, { 7, 7 } } },
			{ name = "_BOOK_3", key = "%_BOOK_3", value = { { 5, 7 }, { 7, 7 } } },
			{ name = "_SPRAY_1", key = "%_SPRAY_1", value = { { 1, 6 }, { 7, 7 } } },
			{ name = "_SECRET_ITEM_1", key = "%_SECRET_ITEM_1", value = { { 0, 5 }, { 7, 7 } } },
			{ name = "_SECRET_ITEM_2", key = "%_SECRET_ITEM_2", value = { { 1, 5 }, { 7, 7 } } },
			{ name = "_SECRET_ITEM_3", key = "%_SECRET_ITEM_3", value = { { 2, 5 }, { 7, 7 } } },
			{ name = "_SECRET_ITEM_4", key = "%_SECRET_ITEM_4", value = { { 3, 5 }, { 7, 7 } } },
		}
	},
	{
		day = 5,
		items = {
			{ name = "_DAMAGED", key = "%_DAMAGED", value = { [0] = { 3, 6 }, { 4, 6 } } },
			{ name = "_FOOD", key = "%_FOOD", value = { { 0, 7 } } },
			{ name = "_COIN_1", key = "%_COIN_1_DAY5", value = { { 0, 6 }, { 7, 7 } } },
			{ name = "_COIN_2", key = "%_COIN_2_DAY5", value = { { 0, 6 }, { 7, 7 } } },
			{ name = "_COIN_3", key = "%_COIN_3_DAY5", value = { { 0, 6 }, { 7, 7 } } },
			{ name = "_COIN_4", key = "%_COIN_4_DAY5", value = { { 0, 6 }, { 7, 7 } } },
			{ name = "_COIN_5", key = "%_COIN_5_DAY5", value = { { 0, 6 }, { 7, 7 } } },
		}
	},
	{
		day = 6,
		items = {
			{ name = "_DAMAGED", key = "%_DAMAGED", value = { [0] = { 3, 6 }, { 4, 6 } } },
		}
	},
	{
		day = 7,
		items = {
			{ name = "_DAMAGED", key = "%_DAMAGED", value = { [0] = { 3, 6 }, { 4, 6 } } },
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
			local nCounter = 0
			if item.counter then nCounter = EvaluateKeyInt( item.counter ) end
			GetMasterLevel():GetMainUI():SetLabel( i - 1, res[1], res[2], nCounter )
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

function GetLabelCounter( name )
	local labels = g_labels[CurDay()]
	local i = labels.itemtbl[name]
	if i then
		local item = labels.items[i]
		if not item.counter then return 0 end
		return EvaluateKeyInt( item.counter )
	end
	return 0
end

function SetLabelKey( name, value )
	local labels = g_labels[CurDay()]
	if not labels then return end
	local i = labels.itemtbl[name]
	if i then
		local item = labels.items[i]
		SetKeyInt( item.key, value )
		local res = item.value[value]
		if res then
			local nCounter = 0
			if item.counter then nCounter = EvaluateKeyInt( item.counter ) end
			GetMasterLevel():GetMainUI():SetLabel( i - 1, res[1], res[2], nCounter )
		else
			GetMasterLevel():GetMainUI():SetLabel( i - 1, -1, -1 )
		end
	end
end

function SetLabelCounter( name, value )
	local labels = g_labels[CurDay()]
	local i = labels.itemtbl[name]
	if i then
		local item = labels.items[i]
		if not item.counter then return end
		SetKeyInt( item.counter, value )
		GetMasterLevel():GetMainUI():SetLabelCounter( i - 1, value )
	end
end

function RefreshAllLabels()
	GetMasterLevel():GetMainUI():ClearLabels()
	local labels = g_labels[CurDay()]
	if not labels then return end
	labels = labels.items
	for k, v in ipairs( labels ) do
		local res = v.value[EvaluateKeyInt( v.key )]
		if res then
			local nCounter = 0
			if v.counter then nCounter = EvaluateKeyInt( v.counter ) end
			GetMasterLevel():GetMainUI():SetLabel( k - 1, res[1], res[2], nCounter )
		end
	end
end