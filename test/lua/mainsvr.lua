local ddcl = require "ddcl"
ddcl.load_all_module()


return function(svr)
	ddcl.callback(function(self, from, cmd, session, data)
		if (cmd % 100000) == 0 then
			print("callback", self, from, cmd, session, data)
		end
	end)
    ddcl.start(svr, function()
        print("start", svr)
        local start_time = ddcl.now()
        for i = 1, 1000000 do
        	ddcl.send(svr, ddcl.PTYPE_SEND, i, "xxx")
        end

        local end_time = ddcl.now()
        print("space:", svr, end_time - start_time)
    end)
end
