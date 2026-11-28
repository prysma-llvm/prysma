fn bool testLogicalNot()
{
   dec string[] feature = "3.test_logical_not : ";
   call print(ref feature);
   dec bool a = true; 
   dec bool b = !a; 
   if (b == false) { return true; } else { return false; }
   return false;
}
fn int32 main() { call print(call testLogicalNot()); return 0; }
