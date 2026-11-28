fn bool testLogicalOr()
{
   dec string[] feature = "2.test_logical_or : ";
   call print(ref feature);
   dec bool a = true || false; 
   if (a == true) { return true; } else { return false; }
   return false;
}
fn int32 main() { call print(call testLogicalOr()); return 0; }
