use std::cell::RefCell;
use std::ffi::{c_char, CStr, CString};

use serde::{Deserialize, Serialize};

pub mod c_abi;

thread_local! {
    static LAST_ERROR: RefCell<Option<CString>> = RefCell::new(None);
}

pub fn clear_last_error() {
    LAST_ERROR.with(|e| *e.borrow_mut() = None);
}

pub fn set_last_error(msg: impl Into<String>) {
    let cstr = CString::new(msg.into()).ok();
    LAST_ERROR.with(|e| *e.borrow_mut() = cstr);
}

pub fn take_last_error() -> *mut c_char {
    let err: Option<CString> = LAST_ERROR.with(|e| e.borrow_mut().take());
    match err {
        Some(cstr) => cstr.into_raw(),
        None => std::ptr::null_mut(),
    }
}

pub fn to_c_string(s: String) -> *mut c_char {
    match CString::new(s) {
        Ok(cstr) => cstr.into_raw(),
        Err(_) => std::ptr::null_mut(),
    }
}

pub unsafe fn free_c_string(ptr: *mut c_char) {
    if !ptr.is_null() {
        let _ = CString::from_raw(ptr);
    }
}

pub unsafe fn c_str_to_string(ptr: *const c_char) -> Option<String> {
    if ptr.is_null() {
        return None;
    }
    CStr::from_ptr(ptr)
        .to_str()
        .ok()
        .map(|s| s.to_string())
}

// JSON types used across the FFI boundary. The C++ layer passes these as
// JSON strings so we don't need to hand-maintain C structs.

#[derive(Serialize, Deserialize, Debug)]
pub struct AccountInput {
    pub chain_id: u32,
    pub private_key: String,
    pub public_key: String,
    pub address: String,
    pub path: String,
    pub options: Option<WalletOptionsInput>,
}

#[derive(Serialize, Deserialize, Debug)]
pub struct AccountOutput {
    pub chain_id: u32,
    pub private_key: String,
    pub public_key: String,
    pub address: String,
    pub path: String,
    pub options: Option<WalletOptionsOutput>,
}

#[derive(Serialize, Deserialize, Debug)]
pub struct TransactionOutput {
    pub chain_id: u32,
    pub raw: String,
    pub sender: String,
    pub signature: String,
}

#[derive(Serialize, Deserialize, Debug)]
pub struct WalletOptionsInput {
    pub use_legacy_path: bool,
    pub specific: Option<WalletChainOptionsInput>,
}

#[derive(Serialize, Deserialize, Debug)]
pub struct WalletOptionsOutput {
    pub use_legacy_path: bool,
    pub specific: Option<WalletChainOptionsOutput>,
}

#[derive(Serialize, Deserialize, Debug)]
#[serde(tag = "type", rename_all = "PascalCase")]
pub enum WalletChainOptionsInput {
    CustomEth { chain_id: u32 },
    CustomIcp { key_type: String },
}

#[derive(Serialize, Deserialize, Debug)]
#[serde(tag = "type", rename_all = "PascalCase")]
pub enum WalletChainOptionsOutput {
    CustomEth { chain_id: u32 },
    CustomIcp { key_type: String },
}

#[derive(Serialize, Deserialize, Debug)]
#[serde(tag = "type", rename_all = "PascalCase")]
pub enum TransactionOptionsInput {
    Evm { chain_id: u32 },
    Btc { prev_scripts: Vec<String>, input_amounts: Vec<u64> },
    Substrate {
        call: String,
        era: String,
        nonce: u32,
        tip: u64,
        asset_id: Option<String>,
        block_hash: String,
        genesis_hash: String,
        spec_version: u32,
        transaction_version: u32,
        app_id: Option<u32>,
        signed_extensions: Option<Vec<String>>,
    },
    Cosmos {
        chain_id: String,
        account_number: u64,
    },
}

#[derive(Serialize, Deserialize, Debug)]
pub struct BigNumberInput {
    pub value: String,
}

#[derive(Serialize, Deserialize, Debug)]
pub struct BigNumberOutput {
    pub value: String,
}

// ---------------------------------------------------------------------------
// Conversions between JSON types and kos-mobile types
// ---------------------------------------------------------------------------

impl From<WalletChainOptionsInput> for kos_mobile::models::WalletChainOptions {
    fn from(input: WalletChainOptionsInput) -> Self {
        match input {
            WalletChainOptionsInput::CustomEth { chain_id } => {
                kos_mobile::models::WalletChainOptions::CustomEth { chain_id }
            }
            WalletChainOptionsInput::CustomIcp { key_type } => {
                kos_mobile::models::WalletChainOptions::CustomIcp { key_type }
            }
        }
    }
}

impl From<WalletOptionsInput> for kos_mobile::models::WalletOptions {
    fn from(input: WalletOptionsInput) -> Self {
        match input.specific {
            None => kos_mobile::models::new_wallet_options(input.use_legacy_path),
            Some(WalletChainOptionsInput::CustomEth { chain_id }) => {
                kos_mobile::models::new_eth_wallet_options(input.use_legacy_path, chain_id)
            }
            Some(WalletChainOptionsInput::CustomIcp { key_type }) => {
                kos_mobile::models::new_icp_wallet_options(input.use_legacy_path, key_type)
            }
        }
    }
}

impl From<kos_mobile::models::WalletChainOptions> for WalletChainOptionsOutput {
    fn from(input: kos_mobile::models::WalletChainOptions) -> Self {
        match input {
            kos_mobile::models::WalletChainOptions::CustomEth { chain_id } => {
                WalletChainOptionsOutput::CustomEth { chain_id }
            }
            kos_mobile::models::WalletChainOptions::CustomIcp { key_type } => {
                WalletChainOptionsOutput::CustomIcp { key_type }
            }
        }
    }
}

impl From<kos_mobile::models::WalletOptions> for WalletOptionsOutput {
    fn from(input: kos_mobile::models::WalletOptions) -> Self {
        WalletOptionsOutput {
            use_legacy_path: input.use_legacy_path,
            specific: input.specific.map(Into::into),
        }
    }
}

impl From<AccountInput> for kos_mobile::KOSAccount {
    fn from(input: AccountInput) -> Self {
        kos_mobile::KOSAccount {
            chain_id: input.chain_id,
            private_key: input.private_key,
            public_key: input.public_key,
            address: input.address,
            path: input.path,
            options: input.options.map(Into::into),
        }
    }
}

impl From<kos_mobile::KOSAccount> for AccountOutput {
    fn from(acc: kos_mobile::KOSAccount) -> Self {
        AccountOutput {
            chain_id: acc.chain_id,
            private_key: acc.private_key,
            public_key: acc.public_key,
            address: acc.address,
            path: acc.path,
            options: acc.options.map(Into::into),
        }
    }
}

impl From<kos_mobile::KOSTransaction> for TransactionOutput {
    fn from(tx: kos_mobile::KOSTransaction) -> Self {
        TransactionOutput {
            chain_id: tx.chain_id,
            raw: tx.raw,
            sender: tx.sender,
            signature: tx.signature,
        }
    }
}

impl From<TransactionOptionsInput> for kos_mobile::models::TransactionChainOptions {
    fn from(input: TransactionOptionsInput) -> Self {
        use kos_mobile::models::{
            new_bitcoin_transaction_options, new_cosmos_transaction_options,
            new_evm_transaction_options, new_substrate_transaction_options,
        };

        match input {
            TransactionOptionsInput::Evm { chain_id } => new_evm_transaction_options(chain_id),
            TransactionOptionsInput::Btc {
                prev_scripts,
                input_amounts,
            } => new_bitcoin_transaction_options(input_amounts, prev_scripts),
            TransactionOptionsInput::Substrate {
                call,
                era,
                nonce,
                tip,
                asset_id,
                block_hash,
                genesis_hash,
                spec_version,
                transaction_version,
                app_id,
                signed_extensions,
            } => new_substrate_transaction_options(
                call,
                era,
                nonce,
                tip,
                asset_id,
                block_hash,
                genesis_hash,
                spec_version,
                transaction_version,
                app_id,
                signed_extensions,
            ),
            TransactionOptionsInput::Cosmos {
                chain_id,
                account_number,
            } => new_cosmos_transaction_options(chain_id, account_number),
        }
    }
}

// ---------------------------------------------------------------------------
// Helpers used by the C ABI layer
// ---------------------------------------------------------------------------

pub fn parse_account(json: &str) -> Result<kos_mobile::KOSAccount, String> {
    let input: AccountInput = serde_json::from_str(json).map_err(|e| e.to_string())?;
    Ok(input.into())
}

pub fn parse_wallet_options(json: Option<&str>) -> Result<Option<kos_mobile::models::WalletOptions>, String> {
    let json = match json {
        None | Some("") | Some("null") => return Ok(None),
        Some(j) => j,
    };
    let input: WalletOptionsInput = serde_json::from_str(json).map_err(|e| e.to_string())?;
    Ok(Some(input.into()))
}

pub fn parse_transaction_options(
    json: Option<&str>,
) -> Result<Option<kos_mobile::models::TransactionChainOptions>, String> {
    let json = match json {
        None | Some("") | Some("null") => return Ok(None),
        Some(j) => j,
    };
    let input: TransactionOptionsInput = serde_json::from_str(json).map_err(|e| e.to_string())?;
    Ok(Some(input.into()))
}

pub fn account_to_json(acc: kos_mobile::KOSAccount) -> Result<String, String> {
    let output: AccountOutput = acc.into();
    serde_json::to_string(&output).map_err(|e| e.to_string())
}

pub fn transaction_to_json(tx: kos_mobile::KOSTransaction) -> Result<String, String> {
    let output: TransactionOutput = tx.into();
    serde_json::to_string(&output).map_err(|e| e.to_string())
}

pub fn parse_big_number(json: &str) -> Result<kos_mobile::number::BigNumber, String> {
    let input: BigNumberInput = serde_json::from_str(json).map_err(|e| e.to_string())?;
    kos_mobile::number::big_number_new(input.value).map_err(|e| e.to_string())
}

pub fn big_number_to_json(bn: kos_mobile::number::BigNumber) -> Result<String, String> {
    let value = kos_mobile::number::big_number_string(bn);
    let output = BigNumberOutput { value };
    serde_json::to_string(&output).map_err(|e| e.to_string())
}

pub fn decode_hex(s: &str) -> Result<Vec<u8>, String> {
    hex::decode(s).map_err(|e| e.to_string())
}

pub fn encode_hex(bytes: &[u8]) -> String {
    hex::encode(bytes)
}

#[cfg(test)]
mod tests {
    use super::*;
    use kos::test_utils::get_test_mnemonic;

    const ITERATIONS: u32 = 10_000;

    #[test]
    fn test_mnemonic_roundtrip() {
        let mnemonic = get_test_mnemonic();
        assert!(kos_mobile::validate_mnemonic(mnemonic.clone()));

        let account = kos_mobile::generate_wallet_from_mnemonic(mnemonic, 38, 0, None).unwrap();
        let json = account_to_json(account).unwrap();
        assert!(json.contains("klv1usdnywjhrlv4tcyu6stxpl6yvhplg35nepljlt4y5r7yppe8er4qujlazy"));
    }

    #[test]
    fn test_big_number_add() {
        let a = parse_big_number(r#"{"value":"123.45"}"#).unwrap();
        let b = parse_big_number(r#"{"value":"0.55"}"#).unwrap();
        let sum = kos_mobile::number::big_number_add(a, b).unwrap();
        let out = big_number_to_json(sum).unwrap();
        assert!(out.contains("124"));
    }

    #[test]
    fn test_encrypt_decrypt_gcm() {
        let data = "hello world".to_string();
        let password = "secret".to_string();
        let encrypted = kos_mobile::encrypt_with_gcm(data.clone(), password.clone(), ITERATIONS).unwrap();
        let decrypted = kos_mobile::decrypt(encrypted, password, ITERATIONS).unwrap();
        assert_eq!(decrypted, data);
    }

    #[test]
    fn test_encrypt_decrypt_cbc() {
        let data = "cbc test data".to_string();
        let password = "cbc-secret".to_string();
        let encrypted = kos_mobile::encrypt_with_cbc(data.clone(), password.clone(), ITERATIONS).unwrap();
        let decrypted = kos_mobile::decrypt(encrypted, password, ITERATIONS).unwrap();
        assert_eq!(decrypted, data);
    }

    #[test]
    fn test_validate_mnemonic() {
        assert!(kos_mobile::validate_mnemonic(get_test_mnemonic()));
        assert!(!kos_mobile::validate_mnemonic("not a valid mnemonic".to_string()));
    }

    #[test]
    fn test_generate_wallet_from_private_key() {
        let mnemonic = get_test_mnemonic();
        let account = kos_mobile::generate_wallet_from_mnemonic(mnemonic.clone(), 38, 0, None).unwrap();
        let pk = account.private_key.clone();
        let from_pk = kos_mobile::generate_wallet_from_private_key(38, pk, None).unwrap();
        assert_eq!(account.address, from_pk.address);
        assert_eq!(account.public_key, from_pk.public_key);
    }

    #[test]
    fn test_sign_message() {
        let mnemonic = get_test_mnemonic();
        let account = kos_mobile::generate_wallet_from_mnemonic(mnemonic, 38, 0, None).unwrap();
        let signature = kos_mobile::sign_message(account, "deadbeef".to_string(), false).unwrap();
        assert!(!signature.is_empty());
    }

    #[test]
    fn test_big_number_ops() {
        let a = parse_big_number(r#"{"value":"100"}"#).unwrap();
        let b = parse_big_number(r#"{"value":"3"}"#).unwrap();

        let product = kos_mobile::number::big_number_multiply(a.clone(), b.clone()).unwrap();
        assert!(kos_mobile::number::big_number_string(product).contains("300"));

        let div = kos_mobile::number::big_number_divide(a.clone(), b.clone()).unwrap();
        assert!(kos_mobile::number::big_number_string(div).contains("33.33"));

        let zero = parse_big_number(r#"{"value":"0"}"#).unwrap();
        assert!(kos_mobile::number::big_number_is_zero(zero));
    }

    #[test]
    fn test_chain_support() {
        assert!(kos_mobile::is_chain_supported(38));
        assert!(!kos_mobile::is_chain_supported(999999));
        let chains = kos_mobile::get_supported_chains();
        assert!(chains.contains(&38));
    }

    #[test]
    fn test_wallet_chain_options_roundtrip() {
        let input = WalletChainOptionsInput::CustomEth { chain_id: 1 };
        let mobile: kos_mobile::models::WalletChainOptions = input.into();
        let output: WalletChainOptionsOutput = mobile.into();
        match output {
            WalletChainOptionsOutput::CustomEth { chain_id } => assert_eq!(chain_id, 1),
            _ => panic!("expected CustomEth"),
        }
    }

    #[test]
    fn test_transaction_options_parsing() {
        let evm = TransactionOptionsInput::Evm { chain_id: 137 };
        let parsed: kos_mobile::models::TransactionChainOptions = evm.into();
        match parsed {
            kos_mobile::models::TransactionChainOptions::Evm { chain_id } => assert_eq!(chain_id, 137),
            _ => panic!("expected Evm"),
        }
    }

    #[test]
    fn test_get_path_by_chain() {
        let path = kos_mobile::get_path_by_chain(38, 0, false).unwrap();
        assert_eq!(path, "m/44'/690'/0'/0'/0'");
    }
}
