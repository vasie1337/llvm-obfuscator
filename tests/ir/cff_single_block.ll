; PASS: cff
; Single-block function: CFF should be a no-op (requires > 1 block).

; CHECK-LABEL: define i32 @single_block
; CHECK-NOT: cff.dispatch
; CHECK: ret i32

define i32 @single_block(i32 %x) {
  %r = add i32 %x, 42
  ret i32 %r
}
