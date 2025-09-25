;
; Rust code:
;
; #[unsafe(no_mangle)]
; pub fn run(COUNT: i32) -> i32 {
;     // const COUNT: i32 = 10000;
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
;     std::hint::black_box(run(10000));
; }

define i32 @run(i32 %COUNT) unnamed_addr {
start:
  %len.dbg.spill = alloca [4 x i8], align 4
  %COUNT.dbg.spill = alloca [4 x i8], align 4
  %inter = alloca [4 x i8], align 4
  %i = alloca [4 x i8], align 4
  store i32 %COUNT, ptr %COUNT.dbg.spill, align 4
  store i32 0, ptr %i, align 4
  store i32 0, ptr %inter, align 4
  br label %bb1

bb1:
  %_5 = load i32, ptr %i, align 4
  %_4 = icmp slt i32 %_5, %COUNT
  br i1 %_4, label %bb2, label %bb5

bb5:
  %_0 = load i32, ptr %inter, align 4
  ret i32 %_0

bb2:
  %0 = load i32, ptr %i, align 4
  %1 = add i32 %0, 1
  store i32 %1, ptr %i, align 4
  %len = load i32, ptr %inter, align 4
  store i32 %len, ptr %len.dbg.spill, align 4
  %_7 = icmp eq i32 %len, %COUNT
  br i1 %_7, label %bb3, label %bb4

bb4:
  %2 = load i32, ptr %inter, align 4
  %3 = add i32 %2, 1
  store i32 %3, ptr %inter, align 4
  br label %bb1

bb3:
  call void @something(i32 %len)
  br label %bb4
}

declare void @something(i32) #1
