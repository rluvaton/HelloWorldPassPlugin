;
; Rust code:
;
; #[unsafe(no_mangle)]
; pub fn run() -> i32 {
;     const COUNT: i32 = 10000;
; 
;     let mut i = 0;
;     let mut inter = 0;
;     let mut my_cap = COUNT;
; 
;     while i < COUNT {
;         i += 1;
;         let len = inter;
; 
;         if len == my_cap {
;             something(len);
;         }
; 
;         inter += 1;
;     }
; 
;     return inter;
; }
; 
; #[inline(never)]
; fn something(len: i32) {
;     std::hint::black_box(len as i128);
; }
; 
; pub fn main() {
;     std::hint::black_box(run());
; }

define i32 @run() unnamed_addr {
start:
  %len.dbg.spill = alloca [4 x i8], align 4
  %my_cap.dbg.spill = alloca [4 x i8], align 4
  %inter = alloca [4 x i8], align 4
  %i = alloca [4 x i8], align 4
  store i32 10000, ptr %my_cap.dbg.spill, align 4
  store i32 0, ptr %i, align 4
  store i32 0, ptr %inter, align 4
  br label %bb1

bb1:
  %_4 = load i32, ptr %i, align 4
  %_3 = icmp slt i32 %_4, 10000
  br i1 %_3, label %bb2, label %bb5

bb5:
  %_0 = load i32, ptr %inter, align 4
  ret i32 %_0

bb2:
  %0 = load i32, ptr %i, align 4
  %1 = add i32 %0, 1
  store i32 %1, ptr %i, align 4
  %len = load i32, ptr %inter, align 4
  store i32 %len, ptr %len.dbg.spill, align 4
  %_6 = icmp eq i32 %len, 10000
  br i1 %_6, label %bb3, label %bb4

bb4:
  %2 = load i32, ptr %inter, align 4
  %3 = add i32 %2, 1
  store i32 %3, ptr %inter, align 4
  br label %bb1

bb3:
  br label %bb4
}

declare void @opaque() unnamed_addr
