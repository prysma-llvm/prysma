fn bool testLoopNotEqual()
{
    dec string[] feature = "2.test_loop_not_equal : ";
    call print(ref feature);
    dec int32 i = 0; 
    while(i != 5) { aff i = i + 1; }
    if (i == 5) { return true; }
    return false; 
}
fn int32 main() { call print(call testLoopNotEqual()); return 0; }
