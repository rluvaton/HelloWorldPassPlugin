; ModuleID = 'test_good'
source_filename = "test"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "aarch64-unknown-linux-gnu"

declare void @something(i64) #1

define dso_local i64 @run() #0 {
entry:
  ; This is the preheader. All loop-invariant values are defined here.
  ; 'my_cap' is a simple constant, 10000. It is NOT a PHI node.
  br label %loop.header

loop.header:
  ; The PHI nodes for the variables that actually change in each iteration.
  %i = phi i64 [ 0, %entry ], [ %i.next, %loop.latch ]
  %inter.phi = phi i64 [ 0, %entry ], [ %inter.next, %loop.latch ]

  ; The main loop exit condition.
  %exit.cond = icmp ult i64 %i, 10000
  br i1 %exit.cond, label %loop.body, label %loop.exit

loop.body:
  ; 'len' gets its value from the 'inter' phi node.
  %len = add i64 %inter.phi, 0 

  ; CRITICAL DIFFERENCE:
  ; The comparison is now against a simple, loop-invariant constant '10000'.
  ; SCEV can easily analyze this.
  %if.cond = icmp eq i64 %len, 10000
  br i1 %if.cond, label %if.then, label %loop.latch

if.then:
  ; This block is provably unreachable.
  call void @something(i64 %len)
  br label %loop.latch

loop.latch:
  ; The latch is now much simpler. It only needs to increment the
  ; induction variables. There are no PHIs needed for 'my_cap'.
  %inter.next = add nuw nsw i64 %inter.phi, 1
  %i.next = add nuw nsw i64 %i, 1
  br label %loop.header

loop.exit:
  ; The final value of inter, which SCEV will determine is 10000.
  ret i64 %inter.phi
}

attributes #0 = { "frame-pointer"="all" }
attributes #1 = { noinline }