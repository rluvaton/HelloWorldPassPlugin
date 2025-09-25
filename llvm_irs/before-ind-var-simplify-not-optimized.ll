; Before "IndVarSimplifyPass on loop %bb2 in function run"
; https://llvm.godbolt.org/z/9fqjrq8GT
;
; For rust code (https://godbolt.org/z/beK8hbesv):
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
  %_45 = icmp sgt i32 %COUNT, 0
  br i1 %_45, label %bb2.preheader, label %bb5

bb2.preheader:                                    ; preds = %start
  br label %bb2

bb5.loopexit:                                     ; preds = %bb4
  %.lcssa = phi i32 [ %1, %bb4 ]
  br label %bb5

bb5:                                              ; preds = %bb5.loopexit, %start
  %inter.sroa.0.0.lcssa = phi i32 [ 0, %start ], [ %.lcssa, %bb5.loopexit ]
  ret i32 %inter.sroa.0.0.lcssa

bb2:                                              ; preds = %bb2.preheader, %bb4
  %i.sroa.0.07 = phi i32 [ %0, %bb4 ], [ 0, %bb2.preheader ]
  %inter.sroa.0.06 = phi i32 [ %1, %bb4 ], [ 0, %bb2.preheader ]
  %0 = add nuw nsw i32 %i.sroa.0.07, 1
  %_7 = icmp eq i32 %inter.sroa.0.06, %COUNT
  br i1 %_7, label %bb3, label %bb4

bb4:                                              ; preds = %bb3, %bb2
  %inter.sroa.0.1 = phi i32 [ %2, %bb3 ], [ %inter.sroa.0.06, %bb2 ]
  %1 = add i32 %inter.sroa.0.1, 1
  %_4 = icmp slt i32 %0, %COUNT
  br i1 %_4, label %bb2, label %bb5.loopexit

bb3:                                              ; preds = %bb2
  tail call void @something(i32 %inter.sroa.0.06)
  %2 = add i32 %inter.sroa.0.06, 666
  br label %bb4
}

declare void @something(i32) #1
