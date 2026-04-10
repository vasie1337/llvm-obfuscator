#![no_std]
#![no_main]

extern "C" {
    fn printf(fmt: *const u8, ...) -> i32;
}

#[inline(never)]
fn classify(x: i32) -> *const u8 {
    match x % 5 {
        0 => b"fizz\0".as_ptr(),
        1 => b"buzz\0".as_ptr(),
        2 => b"fizzbuzz\0".as_ptr(),
        3 => b"none\0".as_ptr(),
        _ => b"other\0".as_ptr(),
    }
}

#[inline(never)]
fn collatz_steps(mut n: i32) -> i32 {
    let mut steps = 0;
    while n != 1 {
        if n % 2 == 0 {
            n /= 2;
        } else {
            n = 3 * n + 1;
        }
        steps += 1;
    }
    steps
}

#[no_mangle]
pub extern "C" fn main() -> i32 {
    unsafe {
        let mut i = 0;
        while i < 10 {
            printf(b"classify(%d) = %s\n\0".as_ptr(), i, classify(i));
            i += 1;
        }
        printf(b"collatz(27) = %d steps\n\0".as_ptr(), collatz_steps(27));
        printf(b"collatz(1000) = %d steps\n\0".as_ptr(), collatz_steps(1000));
    }
    0
}

#[panic_handler]
fn panic(_: &core::panic::PanicInfo) -> ! {
    loop {}
}
