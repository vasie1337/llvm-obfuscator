; PASS: strenc
; Verify that string encryption encrypts the global and creates a ctor.

; The original constant string should no longer appear as plaintext.
; CHECK-NOT: c"Hello, world!\00"
; CHECK: @__strenc_ctor
; CHECK: xor i8

@.str = private constant [14 x i8] c"Hello, world!\00"

define i32 @main() {
  %p = getelementptr [14 x i8], ptr @.str, i64 0, i64 0
  %r = call i32 @puts(ptr %p)
  ret i32 0
}

declare i32 @puts(ptr)
