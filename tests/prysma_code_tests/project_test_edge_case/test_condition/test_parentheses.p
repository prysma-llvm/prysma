fn bool testParentheses() {
    dec string[] feature = "7.test_parentheses : ";
    call print(ref feature);
    dec int32 a = 10; dec float b = 25.5;
    if ((a == 10) && (b == 25.5)) { return true; } else { return false; }
    return false;
}
fn int32 main() { call print(call testParentheses()); return 0; }
