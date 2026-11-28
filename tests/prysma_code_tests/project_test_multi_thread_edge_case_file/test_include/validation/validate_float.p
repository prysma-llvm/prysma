fn bool testEqualFloat()
{
    dec string[] msg = "15.test_equal_float : ";
    call print(ref msg);
    dec bool r = call isEqualFloat(3.14, 3.14);
    if (r == true) {
        return true;
    }
    return false;
}

fn bool testNotEqualFloat()
{
    dec string[] msg = "16.test_not_equal_float : ";
    call print(ref msg);
    dec bool r = call isEqualFloat(1.0, 2.0);
    if (r == false) {
        return true;
    }
    return false;
}

fn bool testGreaterFloat()
{
    dec string[] msg = "17.test_greater_float : ";
    call print(ref msg);
    dec bool r = call isGreaterFloat(9.5, 2.3);
    if (r == true) {
        return true;
    }
    return false;
}

fn bool testLessFloat()
{
    dec string[] msg = "18.test_less_float : ";
    call print(ref msg);
    dec bool r = call isLessFloat(0.5, 1.0);
    if (r == true) {
        return true;
    }
    return false;
}

fn bool testFloatComparisonChain()
{
    dec string[] msg = "19.test_float_comparison_chain : ";
    call print(ref msg);
    dec float a = 5.0;
    dec float b = 10.0;
    dec float c = 5.0;
    dec bool ab = call isLessFloat(a, b);
    dec bool bc = call isGreaterFloat(b, c);
    dec bool ac = call isEqualFloat(a, c);
    if (ab == true) {
        if (bc == true) {
            if (ac == true) {
                return true;
            }
        }
    }
    return false;
}

fn void runFloatTests()
{
    call print(call testEqualFloat()); call backSlashN();
    call print(call testNotEqualFloat()); call backSlashN();
    call print(call testGreaterFloat()); call backSlashN();
    call print(call testLessFloat()); call backSlashN();
    call print(call testFloatComparisonChain()); call backSlashN();
}
