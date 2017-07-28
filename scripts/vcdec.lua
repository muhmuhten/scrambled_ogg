local ffi = require 'ffi'

local function decode_int(s, i)
	return ffi.cast("int *", s:sub(i, i+3))[0], i+4
end

local function decode_str(s, i)
	local len, j = decode_int(s, i)
	return s:sub(j, j+len-1), j+len
end

local function decode_ary(s, i)
	local len, j = decode_int(s, i)
	local t = {}
	for k=1,len do
		t[k], j = decode_str(s, j)
	end
	return t, j
end

local function decode(s)
	local e, i = decode_str(s, 17)
	local t, j = decode_ary(s, i)
	return e, t
end

local e, t = decode(io.read "*a")
print "require 'vcenc'"
for k=0,#t do
	print(string.format("%q", t[k] or e))
end
print "{io.write}"
