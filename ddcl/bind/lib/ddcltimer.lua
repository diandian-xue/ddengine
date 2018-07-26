local c = require "lddcl.timer"

return function(M) ---- start fn


function M.now()
	return c.now()
end

function M.systime()
	return c.systime()
end






end --------- end fn