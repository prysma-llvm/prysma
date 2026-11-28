fn bool testLogicalNotFalse()
{
   dec string[] feature = "4.test_logical_not_false : ";
   call print(ref feature);
   dec bool a = !false; 
   if (a == true) { return true; } else { return false; }
   return false;
}
fn int32 main() { call print(call testLogicalNotFalse()); return 0; }
