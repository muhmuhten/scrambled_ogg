local ffi = require 'ffi'

local function encode64(j)
	return ffi.string(ffi.new("uint64_t[1]", j), 8)
end

local function encode32(j)
	return ffi.string(ffi.new("uint32_t[1]", j), 4)
end

return function (e)
	local s, j = {"\3vorbis", encode32(#e), e}, 0
	local function append(t)
		if type(t) ~= "string" then
			s[4] = encode32(j)
			local str = table.concat(s)
			local out = {unpack(t, 2)}
			out[#out+1] = encode64(#str+1)
			out[#out+1] = str
			out[#out+1] = "\1"
			return t[1](unpack(out))
		end

		s[2*j+5] = encode32(#t)
		s[2*j+6] = t
		j = j+1
		return append
	end
	return append
end
