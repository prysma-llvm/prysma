fn bool testFloatEqual() {
    dec string[] feature = "4.test_float_equal : ";
    call print(ref feature);
    dec float a = 25.5;
    if (a == 25.5) { return true; } else { return false; }
    return false;
}
fn int32 main() { call print(call testFloatEqual()); return 0; }
