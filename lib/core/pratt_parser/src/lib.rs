/*
THE KNOWLEDGE IS PROVIDED 'AS IS', WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF UNLEASHING INDESCRIBABLE HORRORS THAT SHATTER YOUR PSYCHE AND SET YOUR MIND ADRIFT IN THE UNKNOWABLY INFINITE COSMOS.
Yes that is like the MIT license, go read https://doc.rust-lang.org/nomicon/
It's required because YOUR UNSAFE WITH YOUR C++ YOUR EVIL WITH YOUR MEMORY MANAGEMENT (JK)
You cannot use free() or delete on the strings this gives you.
If you do, the universe (or at least your heap) will explode in a flame of glory.
ONLY 32 maximum varibles per equation. 
This is because it's a fixed-size array for variables (vars: [(&str, f64); 32]).
So to use this theres a few things you can call
ya got input_equation(eq) Gives you the steps and answer, no varibles.
ya got solve_with_context(eq, vars) gives you the steps and answers, with varibles.
ya got emit_ir(eq) gives you the steps and stuff, no answer.
ya got eval(equation, vars) gives you the answer with no steps, pass NULL after the comma and space to have no varibles.s
you got one more, not sure if you need it free_result(ptr) this just cleans the memory.

Examples:

input_equation("2 + 3 * 4") should do
step 1: 3 * 4 = 12
step 2: 2 + 12 = 14
result: 14

solve_with_context("a * b + 10", "a=5, b=2") should do
step 1: 5 * 2 = 10
step 2: 10 + 10 = 20
result: 20

emit_ir("(1 + 2) * (3 + 4)") shoudl do
order=0 priority=3 1 + 2 = 3
order=1 priority=3 3 + 4 = 7
order=2 priority=2 3 * 7 = 21

eval("2 + 3 * 4", NULL) should do
"14.0"
eval("a * b + 10", "a=5, b=2") should do
"20.0"
eval("2 @ 3", NULL) should do
"ERROR: unknown char '@'"

free_result is different, you probably wont call it
It's mainly used already internally but it's avalible
All it is, is 

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

so yaeh it's better then your free, but hey it's also scary.

P.S if thou shall to call a function that returns a string, string, or a even bigger string,
THOU are to be responsible and liable for calling free_result on that pointer when thou shall be done.
*/

#![allow(clippy::single_call_fn, reason = "Calling a function once is better then putting it in the call site for readabilty.")]
#![allow(clippy::min_ident_chars, reason = "Short identifiers like i, j, x, y, p (for point/path), or id are industry standard.")]
#![allow(clippy::implicit_return, reason = "I do not need 50 kilobytes of return in my source that is not idiomatic.")]
#![allow(clippy::redundant_closure_for_method_calls, reason = "If I used this, then it would make my code thicker, and would just make it look way worse.")]
#![allow(clippy::not_unsafe_ptr_arg_deref, reason = "It's safe to derefrence it, it wasn't required to be marked unsafe by the start.")]
#![allow(clippy::float_cmp, reason = "We allow this for the tests.")]
#![allow(clippy::cast_precision_loss, reason = "It's not NASA.")]

#![no_std]

pub mod ffi;
pub mod tests;

extern crate alloc;

use core::ptr;
use core::ffi::{CStr, c_char};
use core::arch::asm;
use alloc::string::String;
use alloc::vec::Vec;
use alloc::boxed::Box;
use ryu;

#[global_allocator]
static ALLOC: dlmalloc::GlobalDlmalloc = dlmalloc::GlobalDlmalloc;

#[cfg(not(test))]
#[panic_handler]
//Panic handler, very.. different, will crash the app if it reachs this.
pub fn panic(_info: &core::panic::PanicInfo) -> ! {
    //core::intrinsics::abort() Is a nightly feature and non stable, we do almost the same here.
    //LLVM cannot gaurentee it is the same so it's non stable, therefore we'd be forced into nightly.
    unsafe {
        //abort instructions.
        #[cfg(any(target_arch = "x86", target_arch = "x86_64"))]
        asm!("ud2"); //x86 crash.

        #[cfg(target_arch = "arm")]
        asm!("udf #0"); //ARM crash.

        #[cfg(target_arch = "riscv64")]
        asm!("unimp"); //RISC-V crash.
        
        #[cfg(target_arch = "aarch64")]
        asm!("brk #0"); //64 Bit arm crash.
    }
    loop {}
}

//Core structs and enums
//This is basically just a rip off of hashbrown's version, saves size to not grab the entire thing.
pub struct Context<'a> {
    vars: [(&'a str, f64); 32],
    len: usize,
}

struct Parser { tokens: Vec<Token>, pos: usize }


#[derive(Debug)]
pub struct Step {
    pub priority: u8,
    pub order:    usize,
    pub op:       &'static str,
    pub lhs:      f64,
    pub rhs:      f64,
    pub result:   f64,
}

#[derive(Debug, Clone)]
pub enum Op { Add, Sub, Mul, Div, Pow }

#[derive(Debug, Clone, PartialEq)]
pub enum Token {
    Num(f64),
    Ident(String),
    Error(String),
    Plus, Minus, Star, Slash, Caret,
    LParen, RParen,
}

#[derive(Debug, Clone)]
pub enum Expr {
    Num(f64),
    Variable(String),
    Binary { op: Op, lhs: Box<Expr>, rhs: Box<Expr> },
}

//Same as the Context struct comment.
impl<'a> Context<'a> {
    pub fn new() -> Self {
        Self {
            vars: [("", 0.0); 32],
            len: 0,
        }
    }

    pub fn insert(&mut self, key: &'a str, val: f64) {
        for i in 0..self.len {
            if self.vars[i].0 == key {
                self.vars[i].1 = val;
                return;
            }
        }
        if self.len < 32 {
            self.vars[self.len] = (key, val);
            self.len += 1;
        }
    }

    pub fn get(&self, key: &str) -> Option<f64> {
        for i in 0..self.len {
            if self.vars[i].0 == key {
                return Some(self.vars[i].1);
            }
        }
        None
    }
}

impl Parser {
    fn new(tokens: Vec<Token>) -> Self { Self { tokens, pos: 0 } }
    fn peek(&self) -> Option<&Token> { self.tokens.get(self.pos) }
    fn eat(&mut self) -> Option<Token> {
        let t = self.tokens.get(self.pos).cloned();
        self.pos += 1;
        t
    }
    //Heres my prioritys again, kind of, but hey it maps it to the proper ones.
    fn lbp(tok: &Token) -> u8 {
        match tok {
            Token::Plus | Token::Minus => 1,
            Token::Star | Token::Slash => 2,
            Token::Caret               => 3,
            _                          => 0,
        }
    }
    //This will see parthesis and similar, and if it sees LParen then it will start a new expression, maybe multithread later.
    fn expr(&mut self, min_bp: u8) -> Result<Expr, String> {
		let mut lhs = match self.eat() {
			Some(Token::Num(n)) => Expr::Num(n),
			Some(Token::Ident(name)) => Expr::Variable(name),
			Some(Token::LParen) => {
				let e = self.expr(0)?;
				match self.eat() {
					Some(Token::RParen) => e,
					Some(tok) => {
						let tok_str = match tok {
							Token::RParen    => ")",
							Token::LParen    => "(",
							Token::Plus      => "+",
							Token::Minus     => "-",
							Token::Star      => "*",
							Token::Slash     => "/",
							Token::Caret     => "^",
							Token::Num(_)    => "number",
							Token::Ident(_)  => "identifier",
							Token::Error(_)  => "error",
						};
						let mut pos_buf = ryu::Buffer::new();
						let mut err = String::from("Error: Expected ')' but found '");
						err.push_str(tok_str);
						err.push_str("' at line 1, col ");
						err.push_str(pos_buf.format(self.pos as f64));
						return Err(err);
					}
					None => return Err(String::from("Error: Unclosed parenthesis. Expected ')' but reached end of input")),
				}
			}
			Some(Token::Minus) => {
				let e = self.expr(10)?;
				Expr::Binary { op: Op::Mul, lhs: Box::new(Expr::Num(-1.0)), rhs: Box::new(e) }
			}
			Some(tok) => {
				let tok_str = match tok {
					Token::RParen    => ")",
					Token::LParen    => "(",
					Token::Plus      => "+",
					Token::Minus     => "-",
					Token::Star      => "*",
					Token::Slash     => "/",
					Token::Caret     => "^",
					Token::Num(_)    => "number",
					Token::Ident(_)  => "identifier",
					Token::Error(_)  => "error",
				};
				let mut pos_buf = ryu::Buffer::new();
				let mut err = String::from("Error: Unexpected token '");
				err.push_str(tok_str);
				err.push_str("' at line 1, col ");
				err.push_str(pos_buf.format(self.pos as f64));
				return Err(err);
			}
			None => return Err(String::from("Error: Empty expression")),
		};

        //Thy magic of programming lives here, and if you like varibles your in the right house.
        loop {
            let bp = self.peek().map_or(0, Self::lbp);
            if bp <= min_bp { break; }
            let tok = self.eat().unwrap();
            let op = match tok {
                Token::Plus  => Op::Add,
                Token::Minus => Op::Sub,
                Token::Star  => Op::Mul,
                Token::Slash => Op::Div,
                Token::Caret => Op::Pow,
                other => {
				    let tok_str = match other {
					    Token::RParen    => ")",
					    Token::LParen    => "(",
					    Token::Plus      => "+",
					    Token::Minus     => "-",
					    Token::Star      => "*",
					    Token::Slash     => "/",
					    Token::Caret     => "^",
					    Token::Num(_)    => "number",
					    Token::Ident(_)  => "identifier",
					    Token::Error(_)  => "error",
				    };
				    let mut pos_buf = ryu::Buffer::new();
				    let mut err = String::from("Error: Expected operator or end of expression, found '");
				    err.push_str(tok_str);
				    err.push_str("' at line 1, col ");
				    err.push_str(pos_buf.format(self.pos as f64));
				    return Err(err);
			    }
            };
            let rbp = if matches!(op, Op::Pow) { bp - 1 } else { bp };
            let rhs = self.expr(rbp)?;
            lhs = Expr::Binary { op, lhs: Box::new(lhs), rhs: Box::new(rhs) };
        }
        Ok(lhs)
    }
}

//Heres my prioritys, I don't like to stay ontop of them.
//These don't come from the char, they come from mapping each operation to it.
impl Op {
    //Yes I know this doesn't match the lbp function, but hey it works so whats wrong?
    fn priority(&self) -> u8 {
        match self {
            Op::Pow => 1,
            Op::Mul | Op::Div => 2,
            Op::Add | Op::Sub => 3,
        }
    }
    //And this is what maps the symbol to the operation used in the function above.
    fn symbol(&self) -> &'static str {
        match self { Op::Add=>"+", Op::Sub=>"-", Op::Mul=>"*", Op::Div=>"/", Op::Pow=>"^" }
    }
}


//Turn my messy strings into something my computer can understand.
pub fn lex(src: &str) -> Vec<Token> {
    let mut tokens = Vec::new();
    let mut chars = src.chars().peekable();
    while let Some(&c) = chars.peek() {
        match c {
            ' ' | '\t' => { chars.next(); }
            '0'..='9' | '.' => {
                let mut s = String::new();
                while chars.peek().is_some_and(|c| c.is_ascii_digit() || *c == '.') {
                    s.push(chars.next().unwrap());
                }
                match parse_f64(&s) {
                    Some(n) => tokens.push(Token::Num(n)),
                    None    => tokens.push(Token::Error(String::from("invalid number"))),
                }
            }
            //Big tasty table, did you know + is plus? I didn't, the computer doesn't either.
            '+' => { chars.next(); tokens.push(Token::Plus); }
            '-' => { chars.next(); tokens.push(Token::Minus); }
            '*' => { chars.next(); tokens.push(Token::Star); }
            '/' => { chars.next(); tokens.push(Token::Slash); }
            '^' => { chars.next(); tokens.push(Token::Caret); }
            '(' => { chars.next(); tokens.push(Token::LParen); }
            ')' => { chars.next(); tokens.push(Token::RParen); }
            'a'..='z' | 'A'..='Z' | '_' => {
                let mut s = String::new();
                while let Some(&c) = chars.peek() {
                    if c.is_ascii_alphanumeric() || c == '_' {
                        s.push(chars.next().unwrap());
                    } else {
                        break;
                    }
                }
                //Eh numbers, idents, same thing in my mind.. oh wait.
                tokens.push(Token::Ident(s));
            }
            _ => {
                let mut buf = [0u8; 5];
                let s = c.encode_utf8(&mut buf);
                let mut err = String::from("unknown char '");
                err.push(c);
                err.push('\'');
                tokens.push(Token::Error(err));
                chars.next();
            }
        }
    }
    tokens
}

pub fn collect_steps(expr: &Expr, out: &mut Vec<Step>, ctx: &Context) -> Result<f64, String> {
    match expr {
        Expr::Num(n) => Ok(*n),

        Expr::Variable(name) => {
            match ctx.get(name) {
                Some(val) => Ok(val),
                None => {
                    let mut err = String::from("ERROR: Missing context for variable '");
                    err.push_str(name);
                    err.push_str("'. Please define '");
                    err.push_str(name);
                    err.push_str("' in the vars string.");
                    Err(err)
                }
            }
        }

        Expr::Binary { op, lhs, rhs } => {
            //Pass the context down the tree, no ? returning it.
            //We don't return it up because the `?` operator can only be applied to values that implement `Try`
            //The compiler says that, not me.
            let l = collect_steps(lhs, out, ctx)?;
            let r = collect_steps(rhs, out, ctx)?;
            
            let result = match op {
                Op::Add => l + r,
                Op::Sub => l - r,
                Op::Mul => l * r,
                Op::Div => l / r,
                Op::Pow => pow(l, r),
            };
            let order = out.len();
            out.push(Step {
                priority: op.priority(),
                order,
                op: op.symbol(),
                lhs: l,
                rhs: r,
                result,
            });
            Ok(result)
        }
    }
}

pub fn parse_vars<'a>(var_str: &'a str, ctx: &mut Context<'a>) {
    let b = var_str.as_bytes();
    let mut i = 0;
    while i < b.len() {
        while i < b.len() && b[i] == b' ' { i += 1; }
        let ks = i;
        while i < b.len() && b[i] != b'=' { i += 1; }
        let ke = i;
        if i >= b.len() { break; }
        i += 1;
        let vs = i;
        while i < b.len() && b[i] != b',' { i += 1; }
        let ve = i;
        i += 1;
        let key = trim_bytes(&var_str[ks..ke]);
        let val = trim_bytes(&var_str[vs..ve]);
        if let Some(v) = parse_f64(val) {
            ctx.insert(key, v);
        }
    }
}

pub fn trim_bytes(s: &str) -> &str {
    let b = s.as_bytes();
    let mut start = 0;
    let mut end = b.len();
    while start < end && b[start] == b' ' { start += 1; }
    while end > start && b[end - 1] == b' ' { end -= 1; }
    &s[start..end]
}

pub fn schedule_with_ctx(src: &str, ctx: &Context) -> Result<(Vec<Step>, f64), String> {
    let tokens = lex(src);
    // We run away from our problems and give you a error.
    for tok in &tokens {
        if let Token::Error(msg) = tok {
            let mut err = String::from("ERROR: ");
            err.push_str(msg);
            return Err(err);
        }
    }
    let mut parser = Parser::new(tokens);
    let expr = parser.expr(0)?;
    let mut steps = Vec::new();
    let root = collect_steps(&expr, &mut steps, ctx)?;
    let mut sorted: Vec<Step> = Vec::with_capacity(steps.len());
    for p in 1..=3_u8 {
        let mut i = 0;
        while i < steps.len() {
            if steps[i].priority == p {
                sorted.push(steps.remove(i));
            } else {
                i += 1;
            }
        }
    }
    steps = sorted;
    Ok((steps, root))
}

pub fn schedule(src: &str) -> Result<(Vec<Step>, f64), String> {
    schedule_with_ctx(src, &Context::new())
}

pub fn to_raw(s: &str) -> *mut c_char {
    let len = s.len() + 1;
    // allocate 8 extra bytes at the front to store the length
    unsafe {
        let layout = core::alloc::Layout::from_size_align_unchecked(len + 8, 8);
        let base = alloc::alloc::alloc(layout);
        if base.is_null() { return core::ptr::null_mut(); }
        // store length in first 8 bytes
        *(base as *mut usize) = len;
        // string starts after the 8 byte header
        let ptr = base.add(8) as *mut c_char;
        core::ptr::copy_nonoverlapping(s.as_ptr(), ptr as *mut u8, s.len());
        *ptr.add(s.len()) = 0;
        ptr
    }
}

//We normalize the equation to make it in a format that we can use later to solve it.
pub fn norm_equation(steps: &[Step], root: f64) -> String {
    let mut out = String::new();
    for (i, s) in steps.iter().enumerate() {
        let mut lhs_buf = ryu::Buffer::new();
        let mut rhs_buf = ryu::Buffer::new();
        let mut res_buf = ryu::Buffer::new();
        let mut i_buf = ryu::Buffer::new();
        out.push_str("step ");
        out.push_str(i_buf.format((i + 1) as f64));
        out.push_str(": ");
        out.push_str(lhs_buf.format(s.lhs));
        out.push(' ');
        out.push_str(s.op);
        out.push(' ');
        out.push_str(rhs_buf.format(s.rhs));
        out.push_str(" = ");
        out.push_str(res_buf.format(s.result));
        out.push('\n');
    }
    let mut root_buf = ryu::Buffer::new();
    out.push_str("result: ");
    out.push_str(root_buf.format(root));
    out
}

//Helper math functions are in here to save binary size instead of importing libm.
//This is math, very confusing, found it from inside of libm and modifyed it a bit.
pub fn pow(base: f64, exp: f64) -> f64 {
    let e = exp as i64;
    if e as f64 == exp && e.abs() < 1024 {
        if e == 0 { return 1.0; }
        let mut result = 1.0_f64;
        let mut b = if e < 0 { 1.0 / base } else { base };
        let mut n = if e < 0 { -e } else { e } as u64;
        while n > 0 {
            if n & 1 == 1 { result *= b; }
            b *= b;
            n >>= 1;
        }
        return result;
    }
    exp_approx(exp * ln_approx(base))
}

pub fn ln_approx(x: f64) -> f64 {
    if x <= 0.0 { return f64::NAN; }
    let mut val = x;
    let mut k = 0i32;
    while val >= 2.0 { val *= 0.5; k += 1; }
    while val < 1.0  { val *= 2.0; k -= 1; }
    let t = val - 1.0;
    let mut result = 0.0_f64;
    let mut term = t;
    let mut sign = 1.0_f64;
    for i in 1..=32 {
        result += sign * term / i as f64;
        term *= t;
        sign = -sign;
    }
    result + k as f64 * core::f64::consts::LN_2
}

pub fn exp_approx(x: f64) -> f64 {
    let k = (x / core::f64::consts::LN_2) as i64;
    let r = x - k as f64 * core::f64::consts::LN_2;
    let mut result = 1.0_f64;
    let mut term = 1.0_f64;
    for i in 1..=32 {
        term *= r / i as f64;
        result += term;
    }
    let bits = result.to_bits();
    f64::from_bits(bits.wrapping_add((k as u64).wrapping_shl(52)))
}

//Custom to avoid core::num::dec2flt::impl$4::from_str.
pub fn parse_f64(s: &str) -> Option<f64> {
    let mut chars = s.chars().peekable();
    let mut int_part = 0_u64;
    let mut frac_part = 0_u64;
    let mut frac_div = 1_u64;
    let mut has_dot = false;

    for c in chars {
        match c {
            '0'..='9' => {
                if has_dot {
                    frac_part = frac_part * 10 + (c as u64 - '0' as u64);
                    frac_div *= 10;
                } else {
                    int_part = int_part * 10 + (c as u64 - '0' as u64);
                }
            }
            '.' => {
                if has_dot { return None; }
                has_dot = true;
            }
            _ => return None,
        }
    }

    Some(int_part as f64 + frac_part as f64 / frac_div as f64)
}
