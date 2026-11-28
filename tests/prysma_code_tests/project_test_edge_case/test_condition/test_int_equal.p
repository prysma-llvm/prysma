fn bool testIntEqual() {
    dec string[] feature = "3.test_int_equal : ";
    call print(ref feature);
    dec int32 a = 10;
    if (a == 10) { return true; } else { return false; }
    return false;
}
fn int32 main() { call print(call testIntEqual()); return 0; }
