; PASS: instsub
; Function with no integer binary ops -- instsub should be a no-op.

; CHECK-LABEL: define float @no_int_ops
; CHECK: fadd float
; CHECK: ret float

define float @no_int_ops(float %a, float %b) {
  %r = fadd float %a, %b
  ret float %r
}
