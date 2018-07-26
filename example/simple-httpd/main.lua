local ENGINE_ROOT = os.getenv("DDENGINE_ROOT")
print(ENGINE_ROOT)
package.path = ENGINE_ROOT .. "/example/simple-httpd/?.lua"

local ddcl = require "lddcl.core"
local cjson = require "cjson"

ddcl.init()

local index = 0
local function timeout()
    ddcl.timeout(5000, timeout)
    ddcl.log("socket count:", ddcl.getall_socket_count())
end

local function send_http_content(fd)
    local content = [[
        <!DOCTYPE html>
        <html> <head> <title>Welcome to ddengine-simple-httpd!</title>
        <style> body { width: 35em; margin: 0 auto; font-family: Tahoma, Verdana, Arial, sans-serif;}</style>
        </head>
        <body><h1>hello world</h1>
        </body>
    ]]
    local head = string.format("HTTP/1.0 200 OK\r\nContent-length: %d\r\n\r\n%s",
        string.len(content), content);
    ddcl.send_socket(fd, head)
end

local function on_fd_accept(fd)
    while true do
        local rd = ddcl.read_socket(fd, 0)
        local fd, cmd, data = ddcl.parse_socket_rsp(rd)
        ddcl.log("read, ", fd, cmd, data)
        if cmd == ddcl.DDCL_SOCKET_ERROR then
            ddcl.log("socket close", fd)
            break
        end
        send_http_content(fd)
        ddcl.close_socket(fd)
        break
    end
end

local function init()
    local fd = ddcl.listen_socket("127.0.0.1", 8000, 1000)
    ddcl.log("listen:", fd);
    ddcl.forward_socket(fd)

    --[[
    while true do
        local data = ddcl.accept_socket(fd)
        local nfd, cmd = ddcl.parse_socket_rsp(data)
        ddcl.log("accept", nfd)
        ddcl.fork(function()
            on_fd_accept(nfd)
        end)
    end
    --]]
end

local function connect_ip()
    local start_ms = ddcl.now()
    local data = ddcl.connect_socket("www.baidu.com", 80)
    local fd, cmd = ddcl.parse_socket_rsp(data)
    while true do
        ddcl.send_socket(fd, "GET / HTTP/1.1\r\nHost: www.baidu.com\r\nUser-Agent: curl/7.47.0\r\nAccept: */*\r\n\r\n")
        ddcl.log("connect fd", fd, cmd, body)
        local rd = ddcl.read_socket(fd, 0)
        local fd, cmd, body = ddcl.parse_socket_rsp(rd)
        if cmd == ddcl.DDCL_SOCKET_ERROR then
            ddcl.log("close socket fd", fd)
            break
        end
        ddcl.log("read fd", fd, cmd, body)
        ddcl.close_socket(fd)
        break
    end
    ddcl.log("connect baidu space:", ddcl.now() - start_ms)
end

ddcl.start_non_worker(function(svr, source)
    ddcl.timeout(500, timeout)
    ddcl.callback(function(svr, source, session, ptype, cmd, data, sz)
        if cmd == ddcl.DDCL_CMD_SOCKET then
            local fd, socket_cmd, data = ddcl.parse_socket_rsp(data)
            if socket_cmd == ddcl.DDCL_SOCKET_ACCEPT then
                ddcl.forward_socket(fd)
            elseif socket_cmd == ddcl.DDCL_SOCKET_READ then
                send_http_content(fd)
                ddcl.close_socket(fd)
            end
            ddcl.log("socket cmd:", fd, socket_cmd, data, ddcl.DDCL_SOCKET_ERROR)
        end
    end)

    ddcl.fork(init)
    for i = 1, 1 do
        ddcl.fork(connect_ip)
    end

    --[==[
    --]==]
    local data = {
        name = "diandian",
        head = 1,
        sex = 1,
    }
    local sc = [[
        local path = ...
        package.path = path
        require "agent"
    ]]
    local agent = ddcl.new_service(sc, package.path)
    ddcl.log("agent new", agent)
    for i = 1, 1 do
        ddcl.fork(function()
            ddcl.log("call agnet:", ddcl.call(agent, "xx"))
            --
        end)
    end
end)
