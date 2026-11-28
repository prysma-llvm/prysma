fn bool testLogicalAnd()
{
   dec string[] feature = "1.test_logical_and : ";
   call print(ref feature);
   dec bool a = true && true; 
   if (a == true) { return true; } else { return false; }
   return false;
}
fn int32 main() { call print(call testLogicalAnd()); return 0; }
