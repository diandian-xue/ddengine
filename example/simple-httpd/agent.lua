
local ddcl = require "lddcl.core"
local cjson = require "cjson"

ddcl.start(function(svr, from)
    ddcl.callback(function()
        ddcl.resp("agent resp")
    end)
    ddcl.log("started self:", svr, from)
end)
