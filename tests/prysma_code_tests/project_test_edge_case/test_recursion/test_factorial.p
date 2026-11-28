fn int32 factorial(arg int32 n)
{
   dec string[] feature = "1.factorial : ";
   call print(ref feature);
   if (n <= 1) {
      return 1;
   }
   dec int32 m = n - 1;
   dec int32 r = call factorial(m);
   return n * r;
}

fn bool testFactorial()
{
   dec string[] feature = "2.test_factorial : ";
   call print(ref feature);
   dec int32 result = call factorial(5);
   if (result == 120) {
      return true;
   } else {
      return false;
   }
   return false;
}

fn int32 main() {
    call print(call testFactorial());
    return 0;
}
