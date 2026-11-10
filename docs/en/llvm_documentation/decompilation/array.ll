; ModuleID = 'Array.c'
source_filename = "Array.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

; Array of 5 integers initialized with the values 1, 2, 3, 4, 5
; Arrays in LLVM are represented as type [N*T] where N is the number of elements and T is the element type
; Two indices are used to access an element of an array, the first is the index for the byte position 00 00 00 00 / 00 00 00 00 / 00 00 00 00/
; we leave the first index at 0 but it can be useful for special cases
; The second is for navigating a complete memory block, index i to access the i-th element of the array
; a getelementptr instruction is used to compute the address of the i-th element of the array, using the two indices to navigate memory
; The mechanism behind array memory access uses a mathematical formula to compute the address of the i-th element of the array,
; the formula is: address of element i = base address of the array + (i * size of the element)
; we can jump to the correct memory addresses using this mathematical formula. But the gep tool takes care of the calculation for us.

@__const.array_example.array = private unnamed_addr constant [5 x i32] [i32 1, i32 2, i32 3, i32 4, i32 5], align 16

define dso_local void @array_example() #0 {
entry:
  %array = alloca [5 x i32], align 16
  call void @llvm.memcpy.p0.p0.i64(ptr align 16 %array, ptr align 16 @__const.array_example.array, i64 20, i1 false)
  %arrayidx = getelementptr inbounds [5 x i32], ptr %array, i64 0, i64 0
  store i32 10, ptr %arrayidx, align 16
  %arrayidx1 = getelementptr inbounds [5 x i32], ptr %array, i64 0, i64 1
  store i32 20, ptr %arrayidx1, align 4
  ret void
}

declare void @llvm.memcpy.p0.p0.i64(ptr noalias nocapture writeonly, ptr noalias nocapture readonly, i64, i1 immarg) #1

attributes #0 = { noinline nounwind optnone uwtable "frame-pointer"="all" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #1 = { nocallback nofree nounwind willreturn memory(argmem: readwrite) }

