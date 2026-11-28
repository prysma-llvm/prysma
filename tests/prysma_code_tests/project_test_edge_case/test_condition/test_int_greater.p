fn bool testIntGreater() {
    dec string[] feature = "1.test_int_greater : ";
    call print(ref feature);
    dec int32 a = 5; 
    if (a > 3) { return true; } else { return false; }
    return false;
}
fn int32 main() { call print(call testIntGreater()); return 0; }
