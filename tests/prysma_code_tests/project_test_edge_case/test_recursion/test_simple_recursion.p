fn int32 factorial(arg int32 n) {
   if (n <= 1) { return 1; }
   dec int32 m = n - 1;
   return n * call factorial(m);
}

fn int32 fibonacci(arg int32 n) {
   if (n <= 1) { return n; }
   dec int32 one = n - 1; dec int32 two = n - 2;
   return call fibonacci(one) + call fibonacci(two);
}

fn bool testSimpleRecursion()
{
   dec string[] feature = "5.test_simple_recursion : ";
   call print(ref feature);
   dec int32 fact = call factorial(5);
   dec int32 fib = call fibonacci(7);
   if ((fact == 120) && (fib == 13)) {
      return true;
   } else {
      return false;
   }
   return false;
}

fn int32 main() {
    call print(call testSimpleRecursion());
    return 0;
}
