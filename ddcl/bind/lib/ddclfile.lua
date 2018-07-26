local c = require "lddcl.file"

return function(M, ...)

function M.full_path(path)
	return c.full_path(path)
end

function M.get_cwd()
	return c.get_cwd()
end

function M.is_absolute_path(s)
	return c.is_absolute_path(s)
end

function M.file_is_exist(s)
	return c.file_is_exist(s)
end

function M.is_dir(s)
	return c.is_dir(s)
end

function M.get_file_size(s)
	return c.get_file_size(s)
end

end