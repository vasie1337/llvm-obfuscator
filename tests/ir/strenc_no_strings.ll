; PASS: strenc
; Module with no eligible string globals -- strenc should be a no-op.

; CHECK-NOT: @__strenc_ctor
; CHECK-LABEL: define i32 @compute
; CHECK: ret i32

@global_int = private constant i32 42

define i32 @compute(i32 %x) {
  %v = load i32, ptr @global_int
  %r = add i32 %x, %v
  ret i32 %r
}
