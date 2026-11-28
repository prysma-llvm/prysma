include "math/math.p";
include "validation/validation.p";

fn int32 main()
{
    dec string[] title = "=== _tests _include ===";
    call print(ref title); call backSlashN();

    dec string[] titleMath = "--- _math ---";
    call print(ref titleMath); call backSlashN();
    call runMathTests();

    dec string[] titleValidation = "--- _validation ---";
    call print(ref titleValidation); call backSlashN();
    call runValidationTests();

    dec string[] msgEnd = "=== _end ===";
    call print(ref msgEnd); call backSlashN();
    return 0;
}
