fn bool testLoopConditionExternal()
{
    dec string[] feature = "1.test_loop_condition_external : ";
    call print(ref feature);
    dec int32 i = 0; 
    dec bool condition = i < 5;
    while(condition) {
        aff i = i + 1; 
        aff condition = i < 5;
    }
    if (i == 5) { return true; }
    return false;
}
fn int32 main() { call print(call testLoopConditionExternal()); return 0; }
