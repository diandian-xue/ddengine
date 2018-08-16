local BIN_PATH = os.getenv("BIN_PATH")

local cpath = {
	string.format("%s/?.dll", BIN_PATH),
	string.format("%s/?.so", BIN_PATH),
}

package.cpath = table.concat(cpath, ";")
package.path = "../../bind/lib/?.lua;./?.lua"

local ddcl = require "ddcl"
ddcl.init()
ddcl.load_all_module()

print(package.cpath)
print(ddcl.full_path("src"))
print(ddcl.get_cwd())
print(ddcl.is_absolute_path(ddcl.full_path("src")))
print(ddcl.file_is_exist("CMakeLists.txt"))
print(ddcl.is_dir("CMakeLists.txt"))
print(ddcl.get_file_size("t.lua"))

local script = [[
    local svr, cpath = ...
    package.path =  "../../bind/lib/?.lua;./?.lua"
    package.cpath = cpath
    print("main cpath cpath", cpath)
    local ddcl = require "ddcl"
	ddcl.load_all_module()

	local tsvr = %d

	ddcl.start(svr, function()
		print("mainsvr start:", svr, tsvr)

		local start_time = ddcl.now()
        for i = 1, 5000000 do
        	ddcl.send(tsvr, ddcl.PTYPE_SEND, i, "xxx")
        end

        local end_time = ddcl.now()
        print("space:" .. tostring(end_time - start_time))
	end)
    --require("mainsvr")(svr)
]]

local ts = [[
	local svr, cpath = ...
	package.path =  "../../bind/lib/?.lua;./?.lua"
    package.cpath = cpath
    print("tsvr cpath:", cpath)
	local ddcl = require "ddcl"
	ddcl.load_all_module()

    ddcl.callback(function(self, from, cmd, session, data)
		if (cmd % 100000) == 0 then
			print("callback", self, from, cmd, session, data)
		end
	end)
    ddcl.start(svr, function()
        print("tsvr start", svr)
    end)
	]]

local tsvr = ddcl.new_service(ts, package.cpath)
local script = string.format(script, tsvr)
local svr = ddcl.new_service_not_worker(script, package.cpath)