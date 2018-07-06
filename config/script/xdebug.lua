function print_info(str)
    if str == "help" then
        print "请输入命令(help):"
        print "\th            帮助信息"
        print "\tc            继续运行"
        print "\tn            单步调试(跳过函数)"
        print "\ts            单步调试(不跳过函数)"
        print "\tp var        打印变量"
        print "\tpal          打印当前函数的内部变量"
        print "\tpau          打印当前函数的外部变量"
        print "\tb file:line  设置断点"
        print "\tbl           列出所有断点"
        print "\tbe num       启用一个断点"
        print "\tbd num       停用一个断点"
        print "\tbt           打印堆栈信息"
        print "\tf level      跳转到某层函数"
    elseif str == "prompt" then
        io.write(">>")
    else
        print(str)
    end
end

function print_args_info(str_op, level, index)
    level = level + 3;  --修正函数调用层级
    if str_op == "singlevalue" then
        local name,value = get_local(level, index);
        local flag = "L";
        if not name then
            name,value = get_upvalue(level, index);
            flag = "U";
        end
        if not name then
            return 0;
        end
        local message = traversal_r(value);
        print(message);
        respond_message(flag.."[0] "..tostring(name).." = "..message)
    elseif str_op == "all" then
        local n = 1;
        local i = 0;
        local message = "";
        while true do
            local name,value = get_local(level, n);
            if not name then
                break;
            end
            if name ~= "(*temporary)" then
                message = message.."L["..tostring(i).."] "..tostring(name).." = "..traversal_r(value).."\n";
                i = i + 1;
            end
            n = n + 1;
        end
        n = 1;
        while true do
            local name,value = get_upvalue(level, n);
            if not name then
                break;
            end
            if name ~= "(*temporary)" then
                message = message.."L["..tostring(i).."] "..tostring(name).." = "..traversal_r(value).."\n";
                i = i + 1;
            end
            n = n + 1;
        end
        print(message);
        respond_message(message)
    elseif str_op == "alllocal" then
        local n = 1;
        local i = 0;
        local message = "";
        while true do
            local name,value = get_local(level, n);
            if not name then
                break;
            end
            if name ~= "(*temporary)" then
                message = message.."L["..tostring(i).."] "..tostring(name).." = "..traversal_r(value).."\n";
                i = i + 1;
            end
            n = n + 1;
        end
        print(message)
        respond_message(message)
    elseif str_op == "allupvalue" then
        local n = 1;
        local i = 0;
        local message = "";
        while true do
            local name,value = get_upvalue(level, n);
            if not name then
                break;
            end
            if name ~= "(*temporary)" then
                message = message.."L["..tostring(i).."] "..tostring(name).." = "..traversal_r(value).."\n";
                i = i + 1;
            end
            n = n + 1;
        end
        print(message);
        respond_message(message)
    end
end

function get_local(level, index)
    return debug.getlocal(level, index);
end

function get_upvalue(level, index)
    local ftable = debug.getinfo(level, "f");
    return debug.getupvalue(ftable.func, index);
end

function print_r(value)
    return print(traversal_r(value, 128));
end

function traversal_r(tbl, num)
    num = (num ~= nil) and num or 64
    local ret_table = {}
    local function insert(v)
        table.insert(ret_table, v);
        if #ret_table > num then
            error()
        end
    end
    local function traversal(e)
        if e == {} or e == nil then
            insert("nil,")
        elseif type(e) == "table" then
            insert("{")
            local maxn = 0
            for i,v in ipairs(e) do
                traversal(v)
                maxn = i
            end
            for k,v in pairs(e) do
                if not (type(k) == "number" and k > 0 and k <= maxn) then
                    if type(k) == "numbrer" then
                        insert("["..k.."] = ")
                    else
                        insert(tostring(k).." = ")
                    end
                    traversal(v)
                end
            end
            insert("}, ")
        elseif type(e) == "string" then
            insert('"'..e..'", ')
        else
            insert(tostring(e)..", ")
        end
    end

    local err = xpcall(function() traversal(tbl) end, function() end)
    if not err then
        table.insert(ret_table, "...");
    end
    return tostring(table.concat(ret_table));
end

