; Rust code:
;
; #[unsafe(no_mangle)]
; pub fn run() -> i32 {
;     const COUNT: i32 = 10000;
;     let mut inter = 0;
;     let mut index = 0;
; 
;     while index < COUNT {
;         index += 1;
; 
;         inter += 1;
;     }
; 
;     return inter;
; }


define i32 @run() unnamed_addr {
start:
  %index = alloca [4 x i8], align 4
  %inter = alloca [4 x i8], align 4
  store i32 0, ptr %inter, align 4
  store i32 0, ptr %index, align 4
  br label %bb1

bb1:
  %_4 = load i32, ptr %index, align 4
  %_3 = icmp slt i32 %_4, 10000
  br i1 %_3, label %bb2, label %bb3

bb3:
  %_0 = load i32, ptr %inter, align 4
  ret i32 %_0

bb2:
  %0 = load i32, ptr %index, align 4
  %1 = add i32 %0, 1
  store i32 %1, ptr %index, align 4
  %2 = load i32, ptr %inter, align 4
  %3 = add i32 %2, 1
  store i32 %3, ptr %inter, align 4
  br label %bb1
}
