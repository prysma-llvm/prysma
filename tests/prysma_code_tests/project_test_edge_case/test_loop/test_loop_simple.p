fn bool testLoopSimple()
{
    dec string[] feature = "3.test_loop : ";
    call print(ref feature);
    dec int32 i = 0;
    while (i < 5) { aff i = i + 1; }
    if (i == 5) { return true; } 
    return false;
}
fn int32 main() { call print(call testLoopSimple()); return 0; }
