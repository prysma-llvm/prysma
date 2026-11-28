fn bool testLoopNested()
{
    dec string[] feature = "4.test_loop_nested : ";
    call print(ref feature);
    dec int32 i = 0; dec int32 j = 0; dec int32 total = 0;
    while(i < 5) {
        aff j = 0;
        while(j < 5) { aff total = total + 1; aff j = j + 1; }
        aff i = i + 1;
    }
    if(total == 25) { return true; }
    return false;
}
fn int32 main() { call print(call testLoopNested()); return 0; }
