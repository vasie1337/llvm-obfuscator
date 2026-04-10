; PASS: mbasub
; Verify that MBA substitution fires: the original plain add/sub/xor
; should be replaced with more complex expressions.

; CHECK-LABEL: define i32 @mba_arith
; CHECK-NOT: add i32 %a, %b
; CHECK-NOT: sub i32 %c, %d
; CHECK-NOT: xor i32 %e, %f
; CHECK: ret i32

define i32 @mba_arith(i32 %a, i32 %b, i32 %c, i32 %d, i32 %e, i32 %f) {
  %sum = add i32 %a, %b
  %diff = sub i32 %c, %d
  %x = xor i32 %e, %f
  %t1 = add i32 %sum, %diff
  %t2 = add i32 %t1, %x
  ret i32 %t2
}
