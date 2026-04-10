; PASS: bcf
; Function declaration (no body) -- BCF should not crash, no transform.

; CHECK: declare i32 @extern_func(i32)
; CHECK-NOT: .bcf_opaque

declare i32 @extern_func(i32)
