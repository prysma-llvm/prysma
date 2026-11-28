fn bool testEqualInteger()
{
    dec string[] msg = "8.test_equal_integer : ";
    call print(ref msg);
    dec bool r = call isEqualInt(42, 42);
    if (r == true) {
        return true;
    }
    return false;
}

fn bool testNotEqualInteger()
{
    dec string[] msg = "9.test_not_equal_integer : ";
    call print(ref msg);
    dec bool r = call isEqualInt(10, 20);
    if (r == false) {
        return true;
    }
    return false;
}

fn bool testGreaterInteger()
{
    dec string[] msg = "10.test_greater_integer : ";
    call print(ref msg);
    dec bool r = call isGreaterInt(100, 50);
    if (r == true) {
        return true;
    }
    return false;
}

fn bool testLessInteger()
{
    dec string[] msg = "11.test_less_integer : ";
    call print(ref msg);
    dec bool r = call isLessInt(3, 99);
    if (r == true) {
        return true;
    }
    return false;
}

fn bool testAbsoluteValue()
{
    dec string[] msg = "12.test_absolute_value : ";
    call print(ref msg);
    dec int32 neg = 0 - 42;
    dec int32 r = call absoluteValue(neg);
    if (r == 42) {
        return true;
    }
    return false;
}

fn bool testMaximum()
{
    dec string[] msg = "13.test_maximum : ";
    call print(ref msg);
    dec int32 r = call maximum(15, 30);
    if (r == 30) {
        return true;
    }
    return false;
}

fn bool testMinimum()
{
    dec string[] msg = "14.test_minimum : ";
    call print(ref msg);
    dec int32 r = call minimum(15, 30);
    if (r == 15) {
        return true;
    }
    return false;
}

fn void runIntegerTests()
{
    call print(call testEqualInteger()); call backSlashN();
    call print(call testNotEqualInteger()); call backSlashN();
    call print(call testGreaterInteger()); call backSlashN();
    call print(call testLessInteger()); call backSlashN();
    call print(call testAbsoluteValue()); call backSlashN();
    call print(call testMaximum()); call backSlashN();
    call print(call testMinimum()); call backSlashN();
}
