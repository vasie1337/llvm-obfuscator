#![no_std]
#![no_main]

extern "C" {
    fn printf(fmt: *const u8, ...) -> i32;
}

#[inline(never)]
fn fib(n: i32) -> i64 {
    if n <= 1 {
        return n as i64;
    }
    fib(n - 1) + fib(n - 2)
}

#[inline(never)]
fn factorial(n: i32) -> i64 {
    if n <= 1 {
        return 1;
    }
    n as i64 * factorial(n - 1)
}

#[no_mangle]
pub extern "C" fn main() -> i32 {
    unsafe {
        let mut i = 0;
        while i < 15 {
            printf(b"fib(%d) = %ld\n\0".as_ptr(), i, fib(i));
            i += 1;
        }
        i = 1;
        while i <= 12 {
            printf(b"%d! = %ld\n\0".as_ptr(), i, factorial(i));
            i += 1;
        }
    }
    0
}

#[panic_handler]
fn panic(_: &core::panic::PanicInfo) -> ! {
    loop {}
}
