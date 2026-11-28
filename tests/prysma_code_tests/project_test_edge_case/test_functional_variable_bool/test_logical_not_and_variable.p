fn bool testLogicalNotAndVariable()
{
   dec string[] feature = "7.test_logical_not_and_variable : ";
   call print(ref feature);
   dec bool a = true; dec bool b = false;
   dec bool d = !a && b;
   if (d == false) { return true; } else { return false; }
   return false;
}
fn int32 main() { call print(call testLogicalNotAndVariable()); return 0; }
