fn bool testLogicalOrVariable()
{
   dec string[] feature = "5.test_logical_or_variable : ";
   call print(ref feature);
   dec bool a = true; dec bool b = false;
   dec bool d = a || b;
   if (d == true) { return true; } else { return false; }
   return false;
}
fn int32 main() { call print(call testLogicalOrVariable()); return 0; }
