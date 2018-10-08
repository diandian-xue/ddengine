local ddcl = require "lddcl.core"
local cjson = require "cjson"
local inspect = require "inspect"

local M = {}
local SVR

local CMD_GET = 1
function M.get(host, port, uri, header)
    local t = {
        host = host,
        port = port,
        uri = uri,
        header = header,
        cmd = CMD_GET,
    }
    local data, sz = ddcl.call(SVR, cjson.encode(t))
    data = ddcl.packstring(data, sz)
    return cjson.decode(data)
end

function M.init()
    SVR = ddcl.new_service([[
        require("http")._init_service()
    ]], "")
end


local CMD = {}

CMD[CMD_GET] = function(param)
    local host = param.host or "127.0.0.1"
    local port = param.port or 80
    local data, sz = ddcl.connect_socket(host, port)
    data = ddcl.packstring(data, sz)
    local fd, cmd = ddcl.parse_socket_rsp(data)

    if not fd then
        return { code = 0}
    end

    local uri = param.uri or "/"
    local req = {
        string.format("GET %s HTTP/1.1", uri),
        string.format("Host:%s:%d", host, port),
        "Connection: close",
        "\r\n",
    }
    local req_s = table.concat(req, "\r\n")
    ddcl.send_socket(fd, req_s)
    local resp = {}
    while true do
        local data, sz = ddcl.read_socket(fd, 0)
        data = ddcl.packstring(data, sz)
        local fd, cmd, body = ddcl.parse_socket_rsp(data)
        if cmd == ddcl.DDCL_SOCKET_ERROR then
            break
        end
        table.insert(resp, body)
    end

    local s = table.concat(resp, "")
    local head
    local header = {}
    local body
    local pos = 1
    while pos < #s do
        local fpos = string.find(s, "\r\n", pos)
        if not fpos then
            break
        end
        local sub = string.sub(s, pos, fpos - 1)
        if not head then
            head = sub
        else
            if #sub > 0 then
                table.insert(header, sub)
            else
                body = string.sub(s, fpos + 2, -1)
                break
            end
        end
        pos = fpos + 2
    end

    if not head then
        return { code = 0 }
    end
    for i, v in ipairs(header) do

    end

    local version, code, msg = string.match(head, "([^ ]+) (%d+) ([^ ]+)")
    local ret ={
        version = version,
        code = tonumber(code),
        msg = msg,
        header = header,
        body = body,
    }
    return ret
end


function M._init_service()
    ddcl.start(function()
        ddcl.callback(function(self, from, session, ptype, cmd, data, sz)
            local fn = CMD[cmd]
            assert(fn, cmd)
            data = ddcl.packstring(data, sz)
            if data then
                data = cjson.decode(data)
            end
            if session > 0 then
                ddcl.resp(cjson.encode(fn(data)))
            else
                fn(data)
            end
        end)
    end)
end

return M
