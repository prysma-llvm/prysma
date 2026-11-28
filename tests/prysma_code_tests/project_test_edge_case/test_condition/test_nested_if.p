fn bool testNestedIf() {
    dec string[] feature = "9.test_nested_if : ";
    call print(ref feature);
    dec int32 x = 50;
    if (x > 10) {
        if (x < 100) { return true; } else { return false; }
    } else { return false; }
    return false;
}
fn int32 main() { call print(call testNestedIf()); return 0; }
