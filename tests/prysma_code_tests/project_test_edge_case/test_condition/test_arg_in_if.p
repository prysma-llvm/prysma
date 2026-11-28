fn bool testArgInIf(arg int32 value) {
    dec string[] feature = "8.test_arg_in_if : ";
    call print(ref feature);
    if (value == 100) { return true; } else { return false; }
    return false;
}
fn int32 main() { call print(call testArgInIf(100)); return 0; }
