fn bool testOrSimple() {
    dec string[] feature = "6.test_or_simple : ";
    call print(ref feature);
    dec int32 a = 0; dec int32 b = 1;
    if (a == 1 || b == 1) { return true; } else { return false; }
    return false;
}
fn int32 main() { call print(call testOrSimple()); return 0; }
