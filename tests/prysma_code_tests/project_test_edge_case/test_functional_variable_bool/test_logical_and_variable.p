fn bool testLogicalAndVariable()
{
   dec string[] feature = "6.test_logical_and_variable : ";
   call print(ref feature);
   dec bool a = true; dec bool b = false;
   dec bool d = a && b;
   if (d == false) { return true; } else { return false; }
   return false;
}
fn int32 main() { call print(call testLogicalAndVariable()); return 0; }
