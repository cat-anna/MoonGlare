
actions = {}

function actions.list_actions() 
    for k,v in paris(actions) do
        if type(v) == "function" then 
            print(k)
        end
    end
end

function actions.execute_action(argument_string) 
    local args = utils.explode(";", argument_string)
    local action_name = args[1]
    assert(action_name)
    table.remove(args, 1)
    assert(actions[action_name])
    actions[action_name](table.unpack(args))
end

function actions.build_package(host_source_path, output_file)
    mount_host_folder(host_source_path, "", {
        generate_resource_id = true,
        store_resource_id = false,
    })
    load_module("assimp_import", {
        root_point = "",
    })
    create_exporter("zip", output_file, "", {
        generate_resource_id = false,
        deflate = false,
    }):StartExport()
    create_exporter("content_info", output_file .. ".info", ""):StartExport()
end
