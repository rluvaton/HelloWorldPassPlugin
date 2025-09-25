
define i32 @foo(i32) {
  %2 = shl nsw i32 %0, 1
  ret i32 %2
}

define i32 @bar(i32, i32) {
  %3 = tail call i32 @foo(i32 %1)
  %4 = shl i32 %3, 1
  %5 = add nsw i32 %4, %0
  ret i32 %5
}

define i32 @baz(i32, i32, i32) {
  %4 = tail call i32 @bar(i32 %0, i32 %1)
  %5 = shl i32 %4, 1
  %6 = mul nsw i32 %2, 3
  %7 = add i32 %6, %0
  %8 = add i32 %7, %5
  ret i32 %8
}

define i32 @bez(i32) {
  %2 = tail call i32 @foo(i32 %0)
  %3 = tail call i32 @bar(i32 %0, i32 %2)
  %4 = add nsw i32 %3, %2
  %5 = tail call i32 @baz(i32 %0, i32 %4, i32 123)
  %6 = add nsw i32 %4, %5
  ret i32 %6
}
