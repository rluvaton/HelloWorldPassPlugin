; ModuleID = 'test'
source_filename = "test"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "aarch64-unknown-linux-gnu"

@COUNT = internal constant i64 10000

declare void @something(i64) #1

define dso_local i64 @run() #0 {
entry:
  br label %loop.header

loop.header:
  %i = phi i64 [ 0, %entry ], [ %i.next, %loop.latch ]
  %inter.phi = phi i64 [ 0, %entry ], [ %inter.next, %loop.latch ]
  %my_cap.phi = phi i64 [ 10000, %entry ], [ %my_cap.next, %loop.latch ]

  %exit.cond = icmp ult i64 %i, 10000
  br i1 %exit.cond, label %loop.body, label %loop.exit

loop.body:
  %len = add i64 %inter.phi, 0
  %if.cond = icmp eq i64 %len, %my_cap.phi
  br i1 %if.cond, label %if.then, label %if.end

if.then:
  call void @something(i64 %len)
  %my_cap.incremented = add i64 %my_cap.phi, 1
  br label %if.end

if.end:
  ; This PHI node correctly merges the value of my_cap from the 'if.then' path
  ; and the 'loop.body' path (which is the "else" path). Both are predecessors.
  %my_cap.latch.val = phi i64 [ %my_cap.incremented, %if.then ], [ %my_cap.phi, %loop.body ]
  br label %loop.latch

loop.latch:
  ; The next value of my_cap is determined by the PHI in if.end
  %my_cap.next = phi i64 [ %my_cap.latch.val, %if.end ]
  
  %inter.next = add nuw nsw i64 %inter.phi, 1
  %i.next = add nuw nsw i64 %i, 1
  br label %loop.header

loop.exit:
  ret i64 %inter.phi
}

attributes #0 = { "frame-pointer"="all" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" }
attributes #1 = { noinline }