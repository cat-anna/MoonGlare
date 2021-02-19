local xml2lua = require("xml2lua")
local json = require("json")
local handler = require("xmlhandler.tree")
local parser = xml2lua.parser(handler)

local function get_name_component(value)
    return {
        class = "name",
        component_id = 0,
        data = {
            value = value
        },
        enabled = true
    }
end

local function component_import_not_supported(_, _, name)
    print("IMPORT NOT SUPPORTED: " .. name)
end

local function point_to_array(v)
    local function _num(x)
        if x ~= nil then
            return tonumber(x)
        end
    end
    return {_num(v.X), _num(v.Y), _num(v.Z), _num(v.W)}
end

local function margin_to_array(v)
    local function _num(x)
        if x ~= nil then
            return tonumber(x)
        end
    end
    return {
        top = _num(v.Top),
        left = _num(v.Left),
        right = _num(v.Right),
        bottom = _num(v.Bottom)
    }
end

local function import_image(component, xml_node)
    component.component_id = 17
    component.class = "image"
    local data = component.data

    return component
end

local function import_rect_transform(component, xml_node)
    component.component_id = 16
    component.class = "rect_transform"
    local data = component.data

    data.align_mode = tonumber(xml_node.AlignMode)
    data.z = tonumber(xml_node.Z)

    data.position = point_to_array(xml_node.Position._attr)
    data.size = point_to_array(xml_node.Size._attr)
    data.margin = margin_to_array(xml_node.Margin._attr)

    return component
end

local component_handlers = {
    RectTransform = import_rect_transform,
    Image = import_image,
    __index = function(t, n)
        return component_import_not_supported
    end
}
setmetatable(component_handlers, component_handlers)

local function process_component(xml_node)
    local att = xml_node._attr

    local r = {
        data = {},
        enabled = att.Enabled == "true",
        active = att.Active == "true"
    }

    local component_name = att.Name
    return component_handlers[component_name](r, xml_node, component_name)
end

local function to_table(t)
    if t == nil then
        return {}
    end
    if #t == 0 then
        return {t}
    end
    return t
end

local function process_xml_node(parent, xml_node)
    parent.children = parent.children or {}
    parent.components = parent.components or {}
    parent.enabled = true

    for i, e in ipairs(to_table(xml_node.Entity)) do
        local child = {}
        table.insert(parent.children, child)
        process_xml_node(child, e)
    end

    for i, c in ipairs(to_table(xml_node.Component)) do
        local c_imported = process_component(c)
        if c_imported then
            table.insert(parent.components, c_imported)
        end
    end

    if xml_node._attr then
        local att = xml_node._attr
        if att.Name then
            table.insert(parent.components, get_name_component(att.Name))
        end
        if att.Enabled then
            parent.enabled = att.Enabled == "true"
        end
    end

    return parent
end

local function convert(file_name, output)
    local f = io.open(file_name)
    parser:parse(f:read("*a"))
    f:close()
    f = nil

    -- print(json.encode(handler.root.Scene.Entities))

    f = io.open(output, "w")
    f:write(json.encode(process_xml_node({}, handler.root.Scene.Entities)))
    f:close()

    -- f = io.open(arg[3], "w")
    -- f:write(json.encode(handler.root.Scene.Entities))
    -- f:close()
end

convert(arg[1], arg[2])
