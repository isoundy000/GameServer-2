function foo(arg)
    local test_arg = arg
    local test_t = {};
    if test_arg > 0 then
        print(test_arg);
        foo(test_arg - 1);
    end
end

foo(5);
foo(6);