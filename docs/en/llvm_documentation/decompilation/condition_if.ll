; ModuleID = 'ConditionIf.c'
source_filename = "ConditionIf.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

@.str = private unnamed_addr constant [3 x i8] c"if\00", align 1 
@.str.1 = private unnamed_addr constant [5 x i8] c"else\00", align 1

; Decompiled source code from ConditionIf.c
; Detailed analysis of the if-else condition to understand the inner workings for the purpose of implementing it in my compiler
define dso_local void @checkCondition() #0 {
entry:
 ; The entry block is the entry point where variable allocations and initializations are done
  %number = alloca i32, align 4 ; allocate a variable number of type i32 (32-bit integer)
  store i32 10, ptr %number, align 4 ; store the value 10 in the variable number
  %0 = load i32, ptr %number, align 4 ; load the value of number into variable %0, which is of type i32
  ; three important instructions alloca, store, and load are used to manage variables in memory
  
  %cmp = icmp slt i32 %0, 20 ; compare the value of number to 20, if number < 20 then cmp = 1 else cmp = 0
  ; icmp is used to compare the two values
  ; the result of the comparison is stored in variable %cmp, which is of type i1 (boolean) 1 or 0 

  br i1 %cmp, label %if.then, label %if.else ; here the br instruction is used for conditional branching
  ; it's like the jmp instruction in assembly, it allows jumping to a label depending on the value of %cmp
  ; if %cmp equals 1 (true) then jump to block if.then, else jump to block if.else
if.then:                                          ; if block serves as a jump flag if the condition is met
  %call = call i32 (ptr, ...) @printf(ptr noundef @.str)
  br label %if.end

if.else:                                          ; else block serves as a jump flag if the condition is not met
  %call1 = call i32 (ptr, ...) @printf(ptr noundef @.str.1)
  br label %if.end

if.end:                                           ; end block for the if-else condition, serves as a jump flag for both previous blocks
  ret void
}

declare i32 @printf(ptr noundef, ...) #1

attributes #0 = { noinline nounwind optnone uwtable "frame-pointer"="all" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #1 = { "frame-pointer"="all" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
