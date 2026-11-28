include "addition.p";
include "subtraction.p";
include "multiplication.p";

fn bool testAddition()
{
    dec string[] msg = "1.test_addition : ";
    call print(ref msg);
    dec int32 r = call add(10, 20);
    if (r == 30) {
        return true;
    }
    return false;
}

fn bool testSubtraction()
{
    dec string[] msg = "2.test_subtraction : ";
    call print(ref msg);
    dec int32 r = call subtract(50, 25);
    if (r == 25) {
        return true;
    }
    return false;
}

fn bool testMultiplication()
{
    dec string[] msg = "3.test_multiplication : ";
    call print(ref msg);
    dec int32 r = call multiply(6, 7);
    if (r == 42) {
        return true;
    }
    return false;
}

fn bool testAdditionFloat()
{
    dec string[] msg = "4.test_addition_float : ";
    call print(ref msg);
    dec float r = call addFloat(1.5, 2.5);
    if (r == 4.0) {
        return true;
    }
    return false;
}

fn bool testSubtractionFloat()
{
    dec string[] msg = "5.test_subtraction_float : ";
    call print(ref msg);
    dec float r = call subtractFloat(10.0, 3.5);
    if (r == 6.5) {
        return true;
    }
    return false;
}

fn bool testMultiplicationFloat()
{
    dec string[] msg = "6.test_multiplication_float : ";
    call print(ref msg);
    dec float r = call multiplyFloat(2.5, 4.0);
    if (r == 10.0) {
        return true;
    }
    return false;
}

fn bool testCombinedExpression()
{
    dec string[] msg = "7.test_combined_expression : ";
    call print(ref msg);
    dec int32 a = call add(5, 3);
    dec int32 b = call multiply(a, 2);
    dec int32 c = call subtract(b, 6);
    if (c == 10) {
        return true;
    }
    return false;
}

fn void runMathTests()
{
    call print(call testAddition()); call backSlashN();
    call print(call testSubtraction()); call backSlashN();
    call print(call testMultiplication()); call backSlashN();
    call print(call testAdditionFloat()); call backSlashN();
    call print(call testSubtractionFloat()); call backSlashN();
    call print(call testMultiplicationFloat()); call backSlashN();
    call print(call testCombinedExpression()); call backSlashN();
}
