#![no_std]
#![no_main]

extern "C" {
    fn printf(fmt: *const u8, ...) -> i32;
}

#[no_mangle]
pub extern "C" fn main() -> i32 {
    unsafe {
        printf(b"Hello, obfuscated Rust!\n\0".as_ptr());
    }
    0
}

#[panic_handler]
fn panic(_: &core::panic::PanicInfo) -> ! {
    loop {}
}
