fn int32 fibonacci(arg int32 n) {
   dec string[] feature = "3.fibonacci : ";
   call print(ref feature);
   if (n <= 1) {
      return n;
   }
   dec int32 one = n - 1;
   dec int32 two = n - 2;
   dec int32 fib_one = call fibonacci(one);
   dec int32 fib_two = call fibonacci(two);
   return fib_one + fib_two;
}

fn bool testFibonacci()
{
   dec string[] feature = "4.test_fibonacci : ";
   call print(ref feature);
   dec int32 result = call fibonacci(7);
   if (result == 13) {
      return true;
   } else {
      return false;
   }
   return false;
}

fn int32 main() {
    call print(call testFibonacci());
    return 0;
}
