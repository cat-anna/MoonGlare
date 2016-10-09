-- simple implementation of stack
Stack = {}

local StackIndex = { }
local StackMT = {
    __index = StackIndex,
--    __newindex = function()
--        error("Attempt to modify stack")
--        return nil
--    end,
}

function StackIndex:push(item)
  self[#self + 1] = item
end

function StackIndex:pop()
  local item = self[#self]
  self[#self] = nil
  return item
end

function StackIndex:top()
  return self[#self]
end

function StackIndex:len()
  return #self
end

function StackIndex:empty()
  return #self == 0
end

function Stack:Create()
  return setmetatable({}, StackMT)
end
