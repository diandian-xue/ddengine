

local M = {}

function M.init()
    local c = require "lddcl.core"
	c.init()
end

function M.load_all_module()
	require("ddclfile")(M)
	require("ddclservice")(M)
	require("ddcltimer")(M)
end

return M