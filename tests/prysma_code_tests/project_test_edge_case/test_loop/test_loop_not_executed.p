fn bool testLoopNotExecuted()
{
    dec string[] feature = "6.test_loop_not_executed : ";
    call print(ref feature);
    dec int32 i = 5;
    while(i < 5) { aff i = i + 1; }
    if(i == 5) { return true; }
    return false;
}
fn int32 main() { call print(call testLoopNotExecuted()); return 0; }
