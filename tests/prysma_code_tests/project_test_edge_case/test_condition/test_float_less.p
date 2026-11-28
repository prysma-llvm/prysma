fn bool testFloatLess() {
    dec string[] feature = "2.test_float_less : ";
    call print(ref feature);
    dec float a = 2.5;
    if (a < 5.0) { return true; } else { return false; }
    return false;
}
fn int32 main() { call print(call testFloatLess()); return 0; }
