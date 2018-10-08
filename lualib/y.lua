local ENGINE_ROOT = os.getenv("DDENGINE_ROOT")
local path = {
    ENGINE_ROOT .. "/lualib/?.lua",
    ENGINE_ROOT .. "/example/simple-gl/?.lua",
}
package.path = table.concat(path, ";")

local ddcl = require "lddcl.core"
local cjson = require "cjson"
local http = require "http"
local inspect = require "inspect"


local function url_encode(s)
    s = string.gsub(s, "([^_^%w%.%- ])", function(c) return string.format("%%%02X", string.byte(c)) end)  
    return string.gsub(s, " ", "+")
end

local function url_decode(s)
    s = string.gsub(s, '%%(%x%x)', function(h) return string.char(tonumber(h, 16)) end)
    return s
end

local WORD = table.concat({...}, " ")
print(WORD)
WORD = url_encode(WORD)

local conf = {
    worker = 1,
    socket = 1,
    timer_ms = 1,
}
ddcl.init(conf)


ddcl.start_non_worker(function(self, from, session, ptype, cmd, data, sz)

    ddcl.callback(function(self, from, session, ptype, cmd, data, sz)
    end)

    http.init()
    local uri = "/openapi.do?keyfrom=youdao111&key=60638690&type=data&doctype=json&version=1.1&q=%s"
    uri = string.format(uri, WORD)
    local c, ret = xpcall(http.get, print, "fanyi.youdao.com", 80, uri, {})
    if not c then
        return os.exit()
    end
    local body = string.sub(ret.body, 5, -6)
    local c, js = xpcall(cjson.decode, print, body)
    if not c then
        return os.exit()
    end
    xpcall(function()
        print(inspect(js.basic or js.translation))
    end, print)
    os.exit()
end)
