extern crate alloc;

use core::ptr;
use core::ffi::{CStr, c_char};
use core::arch::asm;
use alloc::string::String;
use alloc::vec::Vec;
use alloc::boxed::Box;
use ryu;

use crate::*;

///INPUT: A math string (ex, "2 + 3 * 4").
///RETURNS: `mut c_char`, showing solved steps.
//The API that a external executable, or app (Like prysma, yes I did copy paste that from the repo I refuse to spell it).
//I'll explain these lines more then the rest because tho may not know Rust.
//No mangle stops it from mangling the name into something like '_ZN7project14input_equationE'.
#[no_mangle]
//Pub marks it public, extern states we are calling a external language, this case c, fn makes it a function, named input_equation, you know the rest there.
pub extern "C" fn input_equation(equation: *const c_char) -> *mut c_char {
    let src = unsafe {
        //the C string (Unsafe because we don't know it exists) to a &str for Rust.
        match CStr::from_ptr(equation).to_str() {
            Ok(s)  => s,
            Err(_) => return ptr::null_mut(),
        }
    };
    let result_str = match schedule(src) {
        Ok((steps, root)) => norm_equation(&steps, root),
        Err(e)            => e,
    };
    to_raw(&result_str)
}

///INPUT: A math string (ex, "10 / 2").
///RETURNS: `mut c_char`, priority, order, etc.
//No mangle, same thing.
#[no_mangle]
//Same here.
pub extern "C" fn emit_ir(equation: *const c_char) -> *mut c_char {
    let src = unsafe {
        //Same matching.
        match CStr::from_ptr(equation).to_str() {
            Ok(s)  => s,
            Err(_) => return ptr::null_mut(),
        }
    };
    let tokens = lex(src);
    let mut parser = Parser::new(tokens);
    let expr = match parser.expr(0) {
        Ok(e)  => e,
        Err(e) => return to_raw(&e),
    };
    let ctx = Context::new();
    let mut steps = Vec::new();
    let _ = collect_steps(&expr, &mut steps, &ctx);
    let mut out = String::new();
    //This is a replacement for the Write! marco, because it's a chonky boi.
    for s in &steps {
        //The matching ones for each {} thingy (Not a bit its not 1/8 byte).
        let mut lhs_buf = ryu::Buffer::new();
        let mut rhs_buf = ryu::Buffer::new();
        let mut res_buf = ryu::Buffer::new();
        let mut order_buf = ryu::Buffer::new();
        let mut priority_buf = ryu::Buffer::new();
        out.push_str("order=");
        out.push_str(order_buf.format(s.order as f64));
        out.push_str(" priority=");
        out.push_str(priority_buf.format(s.priority as f64));
        out.push(' ');
        out.push_str(lhs_buf.format(s.lhs));
        out.push(' ');
        out.push_str(s.op);
        out.push(' ');
        out.push_str(rhs_buf.format(s.rhs));
        out.push_str(" = ");
        out.push_str(res_buf.format(s.result));
        out.push('\n');
    }
    to_raw(&out)
}

///INPUT: The pointer from the above functions.
///RETURN: Nothing, you get NOTHING, you get less memory leaks but you get NOTHING. 
//Clear that memory (drop it).

#[no_mangle]
pub extern "C" fn free_result(s: *mut c_char) {
    if s.is_null() { return; }
    // SAFETY: s was allocated by to_raw, it's length is stored 8 bytes before the pointer
    unsafe {
        let base = (s as *mut u8).sub(8);
        let len = *(base as *const usize);
        let layout = core::alloc::Layout::from_size_align_unchecked(len + 8, 8);
        alloc::alloc::dealloc(base, layout);
    }
}

///INPUT: equation: The math string you want to solve.
//Example: "a * b + 10"
///INPUT: vars: looks for key = value pairs separated by commas.
//Example: "a=5, b=2"
///RETURNS: `mut c_char`, showing solved steps.
#[no_mangle]
pub extern "C" fn solve_with_context(
    equation: *const c_char,
    vars: *const c_char,
) -> *mut c_char {
    let eq_str  = unsafe { CStr::from_ptr(equation).to_str().unwrap_or("") };
    let var_str = unsafe { CStr::from_ptr(vars).to_str().unwrap_or("") };
    let mut ctx = Context::new();
    parse_vars(var_str, &mut ctx);
    let result_str = match schedule_with_ctx(eq_str, &ctx) {
        Ok((steps, root)) => norm_equation(&steps, root),
        Err(e)            => e,
    };
    to_raw(&result_str)
}

///INPUT: equation: The math string you want to solve.
//Example: "a * b + 10" OR "1 * 2 + 3".
///INPUT: Varibles OR NULL: Pass the key = value pairs, separated by commas, and if no varibles after the comma and space pass "NULL".
//Example: "a=5, b=2" OR "NULL".
///RETURNS: ONE (1), yes ONE. String.
///ALL Errors in this are.. a error string..
#[no_mangle]
pub extern "C" fn eval(
    equation: *const c_char,
    vars: *const c_char,
) -> *mut c_char {
    let eq_str = unsafe {
        match CStr::from_ptr(equation).to_str() {
            Ok(s)  => s,
            Err(_) => return to_raw("Error: Invalid input"),
        }
    };
    let mut ctx = Context::new();
    if !vars.is_null() {
        if let Ok(var_str) = unsafe { CStr::from_ptr(vars).to_str() } {
            parse_vars(var_str, &mut ctx);
        }
    }
    let result = match schedule_with_ctx(eq_str, &ctx) {
        Ok((_, root)) => {
            let mut buf = ryu::Buffer::new();
            String::from(buf.format(root))
        }
        Err(e) => e,
    };
    to_raw(&result)
}
