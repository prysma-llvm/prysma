fn bool testLoopConditionComplex()
{
    dec string[] feature = "5.test_loop_condition_complex : ";
    call print(ref feature);
    dec int32 i = 0; dec int32 j = 10;
    while(i < 5 && j > 5) { aff i = i + 1; aff j = j - 1; }
    if(i == 5 && j == 5) { return true; }
    return false;
}
fn int32 main() { call print(call testLoopConditionComplex()); return 0; }
