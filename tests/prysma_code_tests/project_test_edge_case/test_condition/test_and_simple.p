fn bool testAndSimple() {
    dec string[] feature = "5.test_and_simple : ";
    call print(ref feature);
    dec int32 a = 1; dec int32 b = 1;
    if (a == 1 && b == 1) { return true; } else { return false; }
    return false;
}
fn int32 main() { call print(call testAndSimple()); return 0; }
