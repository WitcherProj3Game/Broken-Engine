function GetTabletest()
local lua_table = {}
lua_table.System = Scripting.System()
lua_table.Physics = Scripting.Physics()

function lua_table:Awake()
end

function lua_table:Start()
end

function lua_table:Update()
	local uid = lua_table.Physics:Raycast(0,0,0,0,0,1,20,0,false)

	lua_table.System:LOG("uid: "..uid)
end

return lua_table
end