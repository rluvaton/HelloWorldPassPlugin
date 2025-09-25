define i32 @run() unnamed_addr {
start:
  %b_limit_ptr = alloca [4 x i8]
  %bptr = alloca [4 x i8]
  %aptr = alloca [4 x i8]
  store i32 0, ptr %aptr
  store i32 0, ptr %bptr
  store i32 1000, ptr %b_limit_ptr
  br label %loop_condition

; break if a >= 1000
loop_condition:
  %a = load i32, ptr %aptr
  %done = icmp uge i32 %a, 1000
  br i1 %done, label %do_ret, label %loop_body1

; if b == b_limit, do some side effect that touches b_limit
loop_body1:
  %b = load i32, ptr %bptr
  %b_limit = load i32, ptr %b_limit_ptr
  ; this is never true; `b_limit` is initialized to 1000 but incremented
  ; with `a`. If `a` is 1000, the loop breaks before this bb. `b_limit`
  ; is never modified unless this is true.
  %should_branch = icmp eq i32 %b, %b_limit
  br i1 %should_branch, label %do_side_effect, label %loop_body2

; this is unreachable
do_side_effect:
  ; call void @opaque() ; commenting this enables the optimization
  %b_limit2 = load i32, ptr %b_limit_ptr
  store i32 9, ptr %b_limit_ptr
  br label %loop_body2

; increment both a and b then unconditionally restart the loop
loop_body2:
  %a2 = load i32, ptr %aptr
  %b2 = load i32, ptr %bptr
  %next_a = add i32 %a2, 1
  %next_b = add i32 %b2, 1
  store i32 %next_a, ptr %aptr
  store i32 %next_b, ptr %bptr
  br label %loop_condition

do_ret:
  ret i32 1
}

declare void @opaque() unnamed_addr