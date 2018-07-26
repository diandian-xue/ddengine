local c = require "lddcl.service"

local co_create = coroutine.create
local co_resume = coroutine.resume
local co_yield = coroutine.yield
local co_running = coroutine.running

local DDCL_MSGPTYPE_SEND = 1
local DDCL_MSGPTYPE_RESP = 2
local DDCL_MSGPTYPE_TIMEOUT = 3

local PTYPE_SEND 	= c.PTYPE_SEND
local PTYPE_RESP 	= c.PTYPE_RESP

local CMD_TEXT 		= c.CMD_TEXT
local CMD_ERROR 	= c.CMD_ERROR
local CMD_TIMEOUT 	= c.CMD_TIMEOUT
local CMD_SOCKET 	= c.CMD_SOCKET

return function(M) ---- start fn

M.PTYPE_SEND 	= PTYPE_SEND
M.PTYPE_RESP 	= PTYPE_RESP
M.CMD_TEXT 		= CMD_TEXT
M.CMD_ERROR 	= CMD_ERROR
M.CMD_TIMEOUT 	= CMD_TIMEOUT
M.CMD_SOCKET 	= CMD_SOCKET

local _self = nil
local _callback = nil
local co_sessions = {}

local function _suspend(co, ret, ...)

end

local function _default_callback(self, from, ptype, cmd, session, data)
	if ptype == PTYPE_SEND then
		if _callback then
			return _callback(self, from, cmd, session, data)
		end
		local co = co_create(_callback)
		_suspend(co, co_resume(co, self, from, cmd, session, data))
	elseif ptype == PTYPE_RESP then
		local co = co_sessions[session]
		if not co then
			print("unknow session %d from %08x", session, from)
		end
		_suspend(co, co_resume(co, data))
	else
		assert(false, "unknow ptype %d from %08x", ptype, from)
	end
end


function M.new_service(script, param)
	return c.new_service(script, param)
end

function M.new_service_not_worker(script, param)
	return c.new_service_not_worker(script, param)
end

function M.exit_service(h)
	return c.exit_service(h)
end

function M.send(h, ptype, cmd, data)
	assert(_self)
	local ret = c.send(h, _self, ptype, cmd, 0, data)
	assert(ret == 0, ret)
end

function M.call(h, ptype, cmd, data)
	assert(_self)
	local ret, session = c.call(h, _self, ptype, cmd, data)
	assert(ret == 0, ret)
end

function M.dispatch(h)
	return c.dispatch(h)
end

function M.dispatch_time_wait(h, ms)
	return c.dispatch_time_wait(h, ms)
end

function M.timeout(ms, fn)

end

function M.fork(fn)
	local ret, session = c.send(_self, _self, PTYPE_SEND, CMD_TEXT, "")
	print(ret, session)
	co_sessions[session] = co_create(fn)
end

function M.callback(fn)
	_callback = fn
end

function M.start(h, fn)
	_self = h
	c.callback(_default_callback)
	co_sessions[0] = co_create(fn)
	c.send(_self, _self, PTYPE_RESP, CMD_TEXT, 0, "")
	c.loop_with_not_worker(h)
end

M.running = co_running


end --------- end of start