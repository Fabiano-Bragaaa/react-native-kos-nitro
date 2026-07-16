use std::ffi::{c_char, c_uchar, c_uint};

use crate::*;

#[no_mangle]
pub extern "C" fn kos_nitro_last_error() -> *mut c_char {
    take_last_error()
}

#[no_mangle]
pub extern "C" fn kos_nitro_free_string(ptr: *mut c_char) {
    unsafe { free_c_string(ptr) }
}

// ---------------------------------------------------------------------------
// Wallet
// ---------------------------------------------------------------------------

#[no_mangle]
pub extern "C" fn kos_nitro_generate_mnemonic(size: i32) -> *mut c_char {
    clear_last_error();
    match kos_mobile::generate_mnemonic(size) {
        Ok(mnemonic) => to_c_string(mnemonic),
        Err(e) => {
            set_last_error(e.to_string());
            std::ptr::null_mut()
        }
    }
}

#[no_mangle]
pub extern "C" fn kos_nitro_validate_mnemonic(mnemonic: *const c_char) -> c_uchar {
    clear_last_error();
    let mnemonic = match unsafe { c_str_to_string(mnemonic) } {
        Some(s) => s,
        None => return 0,
    };
    kos_mobile::validate_mnemonic(mnemonic) as c_uchar
}

#[no_mangle]
pub extern "C" fn kos_nitro_get_path_by_chain(
    chain_id: c_uint,
    index: c_uint,
    use_legacy_path: c_uchar,
) -> *mut c_char {
    clear_last_error();
    let use_legacy_path = use_legacy_path != 0;
    match kos_mobile::get_path_by_chain(chain_id, index, use_legacy_path) {
        Ok(path) => to_c_string(path),
        Err(e) => {
            set_last_error(e.to_string());
            std::ptr::null_mut()
        }
    }
}

#[no_mangle]
pub extern "C" fn kos_nitro_generate_wallet_from_mnemonic(
    mnemonic: *const c_char,
    chain_id: c_uint,
    index: c_uint,
    options_json: *const c_char,
) -> *mut c_char {
    clear_last_error();
    let mnemonic = match unsafe { c_str_to_string(mnemonic) } {
        Some(s) => s,
        None => {
            set_last_error("invalid mnemonic string".to_string());
            return std::ptr::null_mut();
        }
    };
    let options = match parse_wallet_options(unsafe { c_str_to_string(options_json).as_deref() }) {
        Ok(o) => o,
        Err(e) => {
            set_last_error(e);
            return std::ptr::null_mut();
        }
    };

    match kos_mobile::generate_wallet_from_mnemonic(mnemonic, chain_id, index, options) {
        Ok(acc) => match account_to_json(acc) {
            Ok(json) => to_c_string(json),
            Err(e) => {
                set_last_error(e);
                std::ptr::null_mut()
            }
        },
        Err(e) => {
            set_last_error(e.to_string());
            std::ptr::null_mut()
        }
    }
}

#[no_mangle]
pub extern "C" fn kos_nitro_generate_wallet_from_private_key(
    chain_id: c_uint,
    private_key: *const c_char,
    options_json: *const c_char,
) -> *mut c_char {
    clear_last_error();
    let private_key = match unsafe { c_str_to_string(private_key) } {
        Some(s) => s,
        None => {
            set_last_error("invalid private key string".to_string());
            return std::ptr::null_mut();
        }
    };
    let options = match parse_wallet_options(unsafe { c_str_to_string(options_json).as_deref() }) {
        Ok(o) => o,
        Err(e) => {
            set_last_error(e);
            return std::ptr::null_mut();
        }
    };

    match kos_mobile::generate_wallet_from_private_key(chain_id, private_key, options) {
        Ok(acc) => match account_to_json(acc) {
            Ok(json) => to_c_string(json),
            Err(e) => {
                set_last_error(e);
                std::ptr::null_mut()
            }
        },
        Err(e) => {
            set_last_error(e.to_string());
            std::ptr::null_mut()
        }
    }
}

// ---------------------------------------------------------------------------
// Signing
// ---------------------------------------------------------------------------

#[no_mangle]
pub extern "C" fn kos_nitro_sign_transaction(
    account_json: *const c_char,
    raw: *const c_char,
    options_json: *const c_char,
) -> *mut c_char {
    clear_last_error();
    let account_json = match unsafe { c_str_to_string(account_json) } {
        Some(s) => s,
        None => {
            set_last_error("invalid account JSON".to_string());
            return std::ptr::null_mut();
        }
    };
    let account = match parse_account(&account_json) {
        Ok(a) => a,
        Err(e) => {
            set_last_error(e);
            return std::ptr::null_mut();
        }
    };
    let raw = match unsafe { c_str_to_string(raw) } {
        Some(s) => s,
        None => {
            set_last_error("invalid raw transaction string".to_string());
            return std::ptr::null_mut();
        }
    };
    let options = match parse_transaction_options(unsafe { c_str_to_string(options_json).as_deref() }) {
        Ok(o) => o,
        Err(e) => {
            set_last_error(e);
            return std::ptr::null_mut();
        }
    };

    match kos_mobile::sign_transaction(account, raw, options) {
        Ok(tx) => match transaction_to_json(tx) {
            Ok(json) => to_c_string(json),
            Err(e) => {
                set_last_error(e);
                std::ptr::null_mut()
            }
        },
        Err(e) => {
            set_last_error(e.to_string());
            std::ptr::null_mut()
        }
    }
}

#[no_mangle]
pub extern "C" fn kos_nitro_sign_message(
    account_json: *const c_char,
    message_hex: *const c_char,
    legacy: c_uchar,
) -> *mut c_char {
    clear_last_error();
    let account_json = match unsafe { c_str_to_string(account_json) } {
        Some(s) => s,
        None => {
            set_last_error("invalid account JSON".to_string());
            return std::ptr::null_mut();
        }
    };
    let account = match parse_account(&account_json) {
        Ok(a) => a,
        Err(e) => {
            set_last_error(e);
            return std::ptr::null_mut();
        }
    };
    let message_hex = match unsafe { c_str_to_string(message_hex) } {
        Some(s) => s,
        None => {
            set_last_error("invalid message hex string".to_string());
            return std::ptr::null_mut();
        }
    };
    let legacy = legacy != 0;

    match kos_mobile::sign_message(account, message_hex, legacy) {
        Ok(signature) => to_c_string(encode_hex(&signature)),
        Err(e) => {
            set_last_error(e.to_string());
            std::ptr::null_mut()
        }
    }
}

// ---------------------------------------------------------------------------
// Chain support
// ---------------------------------------------------------------------------

#[no_mangle]
pub extern "C" fn kos_nitro_is_chain_supported(chain_id: c_uint) -> c_uchar {
    clear_last_error();
    kos_mobile::is_chain_supported(chain_id) as c_uchar
}

#[no_mangle]
pub extern "C" fn kos_nitro_get_supported_chains() -> *mut c_char {
    clear_last_error();
    let chains = kos_mobile::get_supported_chains();
    match serde_json::to_string(&chains) {
        Ok(json) => to_c_string(json),
        Err(e) => {
            set_last_error(e.to_string());
            std::ptr::null_mut()
        }
    }
}

// ---------------------------------------------------------------------------
// Crypto helpers
// ---------------------------------------------------------------------------

#[no_mangle]
pub extern "C" fn kos_nitro_encrypt_with_gcm(
    data: *const c_char,
    password: *const c_char,
    iterations: c_uint,
) -> *mut c_char {
    crypto_common(kos_mobile::encrypt_with_gcm, data, password, iterations)
}

#[no_mangle]
pub extern "C" fn kos_nitro_encrypt_with_cbc(
    data: *const c_char,
    password: *const c_char,
    iterations: c_uint,
) -> *mut c_char {
    crypto_common(kos_mobile::encrypt_with_cbc, data, password, iterations)
}

#[no_mangle]
pub extern "C" fn kos_nitro_encrypt_with_cfb(
    data: *const c_char,
    password: *const c_char,
    iterations: c_uint,
) -> *mut c_char {
    crypto_common(kos_mobile::encrypt_with_cfb, data, password, iterations)
}

#[no_mangle]
pub extern "C" fn kos_nitro_decrypt(
    data: *const c_char,
    password: *const c_char,
    iterations: c_uint,
) -> *mut c_char {
    crypto_common(kos_mobile::decrypt, data, password, iterations)
}

fn crypto_common(
    f: fn(String, String, u32) -> Result<String, kos_mobile::KOSError>,
    data: *const c_char,
    password: *const c_char,
    iterations: c_uint,
) -> *mut c_char {
    clear_last_error();
    let data = match unsafe { c_str_to_string(data) } {
        Some(s) => s,
        None => {
            set_last_error("invalid data string".to_string());
            return std::ptr::null_mut();
        }
    };
    let password = match unsafe { c_str_to_string(password) } {
        Some(s) => s,
        None => {
            set_last_error("invalid password string".to_string());
            return std::ptr::null_mut();
        }
    };

    match f(data, password, iterations) {
        Ok(out) => to_c_string(out),
        Err(e) => {
            set_last_error(e.to_string());
            std::ptr::null_mut()
        }
    }
}

// ---------------------------------------------------------------------------
// Big number
// ---------------------------------------------------------------------------

#[no_mangle]
pub extern "C" fn kos_nitro_big_number_new(value: *const c_char) -> *mut c_char {
    clear_last_error();
    let json = match unsafe { c_str_to_string(value) } {
        Some(s) => s,
        None => {
            set_last_error("invalid big number value JSON".to_string());
            return std::ptr::null_mut();
        }
    };
    match parse_big_number(&json) {
        Ok(bn) => match big_number_to_json(bn) {
            Ok(out) => to_c_string(out),
            Err(e) => {
                set_last_error(e);
                std::ptr::null_mut()
            }
        },
        Err(e) => {
            set_last_error(e);
            std::ptr::null_mut()
        }
    }
}

#[no_mangle]
pub extern "C" fn kos_nitro_big_number_string(value_json: *const c_char) -> *mut c_char {
    clear_last_error();
    let json = match unsafe { c_str_to_string(value_json) } {
        Some(s) => s,
        None => {
            set_last_error("invalid big number JSON".to_string());
            return std::ptr::null_mut();
        }
    };
    match parse_big_number(&json) {
        Ok(bn) => to_c_string(kos_mobile::number::big_number_string(bn)),
        Err(e) => {
            set_last_error(e);
            std::ptr::null_mut()
        }
    }
}

fn big_number_binary_op<F>(lhs_json: *const c_char, rhs_json: *const c_char, op: F) -> *mut c_char
where
    F: FnOnce(
        kos_mobile::number::BigNumber,
        kos_mobile::number::BigNumber,
    ) -> Result<kos_mobile::number::BigNumber, kos_mobile::KOSError>,
{
    clear_last_error();
    let lhs_json = match unsafe { c_str_to_string(lhs_json) } {
        Some(s) => s,
        None => {
            set_last_error("invalid left big number JSON".to_string());
            return std::ptr::null_mut();
        }
    };
    let rhs_json = match unsafe { c_str_to_string(rhs_json) } {
        Some(s) => s,
        None => {
            set_last_error("invalid right big number JSON".to_string());
            return std::ptr::null_mut();
        }
    };
    let lhs = match parse_big_number(&lhs_json) {
        Ok(bn) => bn,
        Err(e) => {
            set_last_error(e);
            return std::ptr::null_mut();
        }
    };
    let rhs = match parse_big_number(&rhs_json) {
        Ok(bn) => bn,
        Err(e) => {
            set_last_error(e);
            return std::ptr::null_mut();
        }
    };

    match op(lhs, rhs) {
        Ok(result) => match big_number_to_json(result) {
            Ok(json) => to_c_string(json),
            Err(e) => {
                set_last_error(e);
                std::ptr::null_mut()
            }
        },
        Err(e) => {
            set_last_error(e.to_string());
            std::ptr::null_mut()
        }
    }
}

#[no_mangle]
pub extern "C" fn kos_nitro_big_number_add(lhs: *const c_char, rhs: *const c_char) -> *mut c_char {
    big_number_binary_op(lhs, rhs, kos_mobile::number::big_number_add)
}

#[no_mangle]
pub extern "C" fn kos_nitro_big_number_subtract(lhs: *const c_char, rhs: *const c_char) -> *mut c_char {
    big_number_binary_op(lhs, rhs, kos_mobile::number::big_number_subtract)
}

#[no_mangle]
pub extern "C" fn kos_nitro_big_number_multiply(lhs: *const c_char, rhs: *const c_char) -> *mut c_char {
    big_number_binary_op(lhs, rhs, kos_mobile::number::big_number_multiply)
}

#[no_mangle]
pub extern "C" fn kos_nitro_big_number_divide(lhs: *const c_char, rhs: *const c_char) -> *mut c_char {
    big_number_binary_op(lhs, rhs, kos_mobile::number::big_number_divide)
}

#[no_mangle]
pub extern "C" fn kos_nitro_big_number_pow(base: *const c_char, exponent: *const c_char) -> *mut c_char {
    big_number_binary_op(base, exponent, kos_mobile::number::big_number_pow)
}

fn big_number_compare<F>(lhs_json: *const c_char, rhs_json: *const c_char, op: F) -> c_uchar
where
    F: FnOnce(kos_mobile::number::BigNumber, kos_mobile::number::BigNumber) -> bool,
{
    clear_last_error();
    let lhs_json = match unsafe { c_str_to_string(lhs_json) } {
        Some(s) => s,
        None => {
            set_last_error("invalid left big number JSON".to_string());
            return 0;
        }
    };
    let rhs_json = match unsafe { c_str_to_string(rhs_json) } {
        Some(s) => s,
        None => {
            set_last_error("invalid right big number JSON".to_string());
            return 0;
        }
    };
    let lhs = match parse_big_number(&lhs_json) {
        Ok(bn) => bn,
        Err(e) => {
            set_last_error(e);
            return 0;
        }
    };
    let rhs = match parse_big_number(&rhs_json) {
        Ok(bn) => bn,
        Err(e) => {
            set_last_error(e);
            return 0;
        }
    };

    op(lhs, rhs) as c_uchar
}

#[no_mangle]
pub extern "C" fn kos_nitro_big_number_is_equal(lhs: *const c_char, rhs: *const c_char) -> c_uchar {
    big_number_compare(lhs, rhs, kos_mobile::number::big_number_is_equal)
}

#[no_mangle]
pub extern "C" fn kos_nitro_big_number_is_gt(lhs: *const c_char, rhs: *const c_char) -> c_uchar {
    big_number_compare(lhs, rhs, kos_mobile::number::big_number_is_gt)
}

#[no_mangle]
pub extern "C" fn kos_nitro_big_number_is_gte(lhs: *const c_char, rhs: *const c_char) -> c_uchar {
    big_number_compare(lhs, rhs, kos_mobile::number::big_number_is_gte)
}

#[no_mangle]
pub extern "C" fn kos_nitro_big_number_is_lt(lhs: *const c_char, rhs: *const c_char) -> c_uchar {
    big_number_compare(lhs, rhs, kos_mobile::number::big_number_is_lt)
}

#[no_mangle]
pub extern "C" fn kos_nitro_big_number_is_lte(lhs: *const c_char, rhs: *const c_char) -> c_uchar {
    big_number_compare(lhs, rhs, kos_mobile::number::big_number_is_lte)
}

fn big_number_predicate<F>(value_json: *const c_char, op: F) -> c_uchar
where
    F: FnOnce(kos_mobile::number::BigNumber) -> bool,
{
    clear_last_error();
    let json = match unsafe { c_str_to_string(value_json) } {
        Some(s) => s,
        None => {
            set_last_error("invalid big number JSON".to_string());
            return 0;
        }
    };
    match parse_big_number(&json) {
        Ok(bn) => op(bn) as c_uchar,
        Err(e) => {
            set_last_error(e);
            0
        }
    }
}

#[no_mangle]
pub extern "C" fn kos_nitro_big_number_is_zero(value: *const c_char) -> c_uchar {
    big_number_predicate(value, kos_mobile::number::big_number_is_zero)
}

#[no_mangle]
pub extern "C" fn kos_nitro_big_number_is_positive(value: *const c_char) -> c_uchar {
    big_number_predicate(value, kos_mobile::number::big_number_is_positive)
}

#[no_mangle]
pub extern "C" fn kos_nitro_big_number_is_negative(value: *const c_char) -> c_uchar {
    big_number_predicate(value, kos_mobile::number::big_number_is_negative)
}

// ---------------------------------------------------------------------------
// Signer (BIP85 / xpub / ECIES / HMAC)
// ---------------------------------------------------------------------------

#[no_mangle]
pub extern "C" fn kos_nitro_generate_xpub(
    mnemonic: *const c_char,
    passphrase: *const c_char,
    is_mainnet: c_uchar,
    index: c_uint,
) -> *mut c_char {
    sign_call(
        mnemonic,
        passphrase,
        is_mainnet,
        index,
        |m, p, main, i| kos_mobile::signer::generate_xpub(m, p, main, i),
    )
}

#[no_mangle]
pub extern "C" fn kos_nitro_get_xpub_as_string(
    mnemonic: *const c_char,
    passphrase: *const c_char,
    is_mainnet: c_uchar,
    index: c_uint,
) -> *mut c_char {
    clear_last_error();
    let (m, p, main, i) = match parse_signer_args(mnemonic, passphrase, is_mainnet, index) {
        Ok(args) => args,
        Err(e) => {
            set_last_error(e);
            return std::ptr::null_mut();
        }
    };
    match kos_mobile::signer::get_xpub_as_string(&m, &p, main, i) {
        Ok(s) => to_c_string(s),
        Err(e) => {
            set_last_error(e.to_string());
            std::ptr::null_mut()
        }
    }
}

#[no_mangle]
pub extern "C" fn kos_nitro_derive_xpub(
    mnemonic: *const c_char,
    passphrase: *const c_char,
    is_mainnet: c_uchar,
    index: c_uint,
    derivation_path: *const c_char,
) -> *mut c_char {
    clear_last_error();
    let (m, p, main, i) = match parse_signer_args(mnemonic, passphrase, is_mainnet, index) {
        Ok(args) => args,
        Err(e) => {
            set_last_error(e);
            return std::ptr::null_mut();
        }
    };
    let derivation_path = match unsafe { c_str_to_string(derivation_path) } {
        Some(s) => s,
        None => {
            set_last_error("invalid derivation path".to_string());
            return std::ptr::null_mut();
        }
    };
    match kos_mobile::signer::derive_xpub(&m, &p, main, i, &derivation_path) {
        Ok(bytes) => to_c_string(encode_hex(&bytes)),
        Err(e) => {
            set_last_error(e.to_string());
            std::ptr::null_mut()
        }
    }
}

#[no_mangle]
pub extern "C" fn kos_nitro_slip77_master_blinding_key(
    mnemonic: *const c_char,
    passphrase: *const c_char,
    is_mainnet: c_uchar,
    index: c_uint,
) -> *mut c_char {
    sign_call(
        mnemonic,
        passphrase,
        is_mainnet,
        index,
        |m, p, main, i| kos_mobile::signer::slip77_master_blinding_key(m, p, main, i),
    )
}

#[no_mangle]
pub extern "C" fn kos_nitro_sign_ecdsa_recoverable(
    mnemonic: *const c_char,
    passphrase: *const c_char,
    is_mainnet: c_uchar,
    index: c_uint,
    msg_hex: *const c_char,
) -> *mut c_char {
    clear_last_error();
    let (m, p, main, i) = match parse_signer_args(mnemonic, passphrase, is_mainnet, index) {
        Ok(args) => args,
        Err(e) => {
            set_last_error(e);
            return std::ptr::null_mut();
        }
    };
    let msg = match unsafe { c_str_to_string(msg_hex) } {
        Some(s) => match decode_hex(&s) {
            Ok(b) => b,
            Err(e) => {
                set_last_error(e);
                return std::ptr::null_mut();
            }
        },
        None => {
            set_last_error("invalid message hex".to_string());
            return std::ptr::null_mut();
        }
    };
    match kos_mobile::signer::sign_ecdsa_recoverable(&m, &p, main, i, msg) {
        Ok(bytes) => to_c_string(encode_hex(&bytes)),
        Err(e) => {
            set_last_error(e.to_string());
            std::ptr::null_mut()
        }
    }
}

#[no_mangle]
pub extern "C" fn kos_nitro_hmac_sha256(
    mnemonic: *const c_char,
    passphrase: *const c_char,
    is_mainnet: c_uchar,
    index: c_uint,
    derivation_path: *const c_char,
    msg_hex: *const c_char,
) -> *mut c_char {
    clear_last_error();
    let (m, p, main, i) = match parse_signer_args(mnemonic, passphrase, is_mainnet, index) {
        Ok(args) => args,
        Err(e) => {
            set_last_error(e);
            return std::ptr::null_mut();
        }
    };
    let derivation_path = match unsafe { c_str_to_string(derivation_path) } {
        Some(s) => s,
        None => {
            set_last_error("invalid derivation path".to_string());
            return std::ptr::null_mut();
        }
    };
    let msg = match unsafe { c_str_to_string(msg_hex) } {
        Some(s) => match decode_hex(&s) {
            Ok(b) => b,
            Err(e) => {
                set_last_error(e);
                return std::ptr::null_mut();
            }
        },
        None => {
            set_last_error("invalid message hex".to_string());
            return std::ptr::null_mut();
        }
    };
    match kos_mobile::signer::hmac_sha256(&m, &p, main, i, &derivation_path, msg) {
        Ok(bytes) => to_c_string(encode_hex(&bytes)),
        Err(e) => {
            set_last_error(e.to_string());
            std::ptr::null_mut()
        }
    }
}

#[no_mangle]
pub extern "C" fn kos_nitro_ecies_encrypt(
    mnemonic: *const c_char,
    passphrase: *const c_char,
    is_mainnet: c_uchar,
    index: c_uint,
    msg_hex: *const c_char,
) -> *mut c_char {
    ecies_call(mnemonic, passphrase, is_mainnet, index, msg_hex, kos_mobile::signer::ecies_encrypt)
}

#[no_mangle]
pub extern "C" fn kos_nitro_ecies_decrypt(
    mnemonic: *const c_char,
    passphrase: *const c_char,
    is_mainnet: c_uchar,
    index: c_uint,
    msg_hex: *const c_char,
) -> *mut c_char {
    ecies_call(mnemonic, passphrase, is_mainnet, index, msg_hex, kos_mobile::signer::ecies_decrypt)
}

fn sign_call<F>(
    mnemonic: *const c_char,
    passphrase: *const c_char,
    is_mainnet: c_uchar,
    index: c_uint,
    f: F,
) -> *mut c_char
where
    F: FnOnce(&str, &str, bool, u32) -> Result<Vec<u8>, kos_mobile::signer::LdError>,
{
    clear_last_error();
    let (m, p, main, i) = match parse_signer_args(mnemonic, passphrase, is_mainnet, index) {
        Ok(args) => args,
        Err(e) => {
            set_last_error(e);
            return std::ptr::null_mut();
        }
    };
    match f(&m, &p, main, i) {
        Ok(bytes) => to_c_string(encode_hex(&bytes)),
        Err(e) => {
            set_last_error(e.to_string());
            std::ptr::null_mut()
        }
    }
}

fn ecies_call<F>(
    mnemonic: *const c_char,
    passphrase: *const c_char,
    is_mainnet: c_uchar,
    index: c_uint,
    msg_hex: *const c_char,
    f: F,
) -> *mut c_char
where
    F: FnOnce(&str, &str, bool, u32, Vec<u8>) -> Result<Vec<u8>, kos_mobile::signer::LdError>,
{
    clear_last_error();
    let (m, p, main, i) = match parse_signer_args(mnemonic, passphrase, is_mainnet, index) {
        Ok(args) => args,
        Err(e) => {
            set_last_error(e);
            return std::ptr::null_mut();
        }
    };
    let msg = match unsafe { c_str_to_string(msg_hex) } {
        Some(s) => match decode_hex(&s) {
            Ok(b) => b,
            Err(e) => {
                set_last_error(e);
                return std::ptr::null_mut();
            }
        },
        None => {
            set_last_error("invalid message hex".to_string());
            return std::ptr::null_mut();
        }
    };
    match f(&m, &p, main, i, msg) {
        Ok(bytes) => to_c_string(encode_hex(&bytes)),
        Err(e) => {
            set_last_error(e.to_string());
            std::ptr::null_mut()
        }
    }
}

fn parse_signer_args(
    mnemonic: *const c_char,
    passphrase: *const c_char,
    is_mainnet: c_uchar,
    index: c_uint,
) -> Result<(String, String, bool, u32), String> {
    let mnemonic = unsafe { c_str_to_string(mnemonic) }
        .ok_or_else(|| "invalid mnemonic".to_string())?;
    let passphrase = unsafe { c_str_to_string(passphrase) }
        .ok_or_else(|| "invalid passphrase".to_string())?;
    Ok((mnemonic, passphrase, is_mainnet != 0, index))
}
