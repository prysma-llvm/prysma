include "tools/tools.p";
include "validate_integer.p";
include "validate_float.p";

fn void runValidationTests()
{
    call runIntegerTests();
    call runFloatTests();
}
