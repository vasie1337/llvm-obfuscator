#![no_std]
#![no_main]

extern "C" {
    fn printf(fmt: *const u8, ...) -> i32;
}

#[inline(never)]
fn add(a: i32, b: i32) -> i32 { a + b }
#[inline(never)]
fn sub(a: i32, b: i32) -> i32 { a - b }
#[inline(never)]
fn xor(a: i32, b: i32) -> i32 { a ^ b }
#[inline(never)]
fn and(a: i32, b: i32) -> i32 { a & b }
#[inline(never)]
fn or(a: i32, b: i32) -> i32 { a | b }

#[inline(never)]
fn compute(x: i32) -> i32 {
    let mut result = 0i32;
    let mut i = 0;
    while i < x {
        result = add(result, i);
        result = xor(result, i.wrapping_mul(3));
        result = or(result, i & 0xFF);
        result = and(result, 0x7FFFFFFF);
        i += 1;
    }
    result
}

#[no_mangle]
pub extern "C" fn main() -> i32 {
    unsafe {
        printf(b"add(10, 20) = %d\n\0".as_ptr(), add(10, 20));
        printf(b"sub(50, 17) = %d\n\0".as_ptr(), sub(50, 17));
        printf(b"xor(0xAA, 0x55) = 0x%X\n\0".as_ptr(), xor(0xAA, 0x55));
        printf(b"and(0xFF, 0x0F) = 0x%X\n\0".as_ptr(), and(0xFF, 0x0F));
        printf(b"or(0xF0, 0x0F) = 0x%X\n\0".as_ptr(), or(0xF0, 0x0F));
        printf(b"compute(100) = %d\n\0".as_ptr(), compute(100));
    }
    0
}

#[panic_handler]
fn panic(_: &core::panic::PanicInfo) -> ! {
    loop {}
}
