; ModuleID = 'BoucleWhile.c'
source_filename = "BoucleWhile.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

define dso_local void @loop() #0 {
entry:
; initialize variable index to 0, allocate memory for index and store the value 0 in this memory
  %index = alloca i32, align 4
  store i32 0, ptr %index, align 4
  br label %while.cond

while.cond:                                    
  %0 = load i32, ptr %index, align 4 ; load the value of index into virtual register %0, which is of type i32
  %cmp = icmp slt i32 %0, 10 ; compare the value of index to 10, if index < 10 then cmp = 1 else cmp = 0
  br i1 %cmp, label %while.body, label %while.end ; here we use the br instruction for conditional branching
  ; if %cmp equals 1 (true) then jump to block while.body, else jump to block while.end 

while.body:                                     
  %1 = load i32, ptr %index, align 4
  %inc = add nsw i32 %1, 1
  store i32 %inc, ptr %index, align 4
  br label %while.cond, !llvm.loop !6

while.end:                                       
  ret void
}

attributes #0 = { noinline nounwind optnone uwtable "frame-pointer"="all" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }

