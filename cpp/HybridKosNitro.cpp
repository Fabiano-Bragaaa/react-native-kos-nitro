#include "HybridKosNitro.hpp"
#include "KosNitroJson.hpp"
#include "../rust/kos-nitro/include/kos_nitro.h"

#include <NitroModules/ArrayBuffer.hpp>
#include <stdexcept>
#include <type_traits>
#include <variant>

namespace margelo::nitro::kosnitro {

using namespace KosNitroJson;

namespace {

int hexCharValue(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return 0;
}

} // namespace

std::string HybridKosNitro::callString(char* raw) {
    if (raw == nullptr) {
        std::string err = getLastError();
        if (!err.empty()) {
            throw std::runtime_error("kos-nitro error: " + err);
        }
        throw std::runtime_error("kos-nitro returned null without an error message");
    }
    std::string result(raw);
    kos_nitro_free_string(raw);
    return result;
}

std::string HybridKosNitro::getLastError() {
    char* err = kos_nitro_last_error();
    if (err == nullptr) return "";
    std::string result(err);
    kos_nitro_free_string(err);
    return result;
}

std::vector<uint8_t> HybridKosNitro::hexToBytes(const std::string& hex) {
    std::vector<uint8_t> out;
    if (hex.size() % 2 != 0) return out;
    out.reserve(hex.size() / 2);
    for (size_t i = 0; i < hex.size(); i += 2) {
        uint8_t byte = static_cast<uint8_t>((hexCharValue(hex[i]) << 4) | hexCharValue(hex[i + 1]));
        out.push_back(byte);
    }
    return out;
}

std::shared_ptr<ArrayBuffer> HybridKosNitro::hexToArrayBuffer(const std::string& hex) {
    std::vector<uint8_t> bytes = hexToBytes(hex);
    return ArrayBuffer::copy(bytes);
}

// ---------------------------------------------------------------------------
// JSON builders
// ---------------------------------------------------------------------------

std::string HybridKosNitro::buildWalletChainOptionsJson(const std::variant<CustomEthWalletOptions, CustomIcpWalletOptions>& specific) {
    return std::visit([](auto&& arg) -> std::string {
        using T = std::decay_t<decltype(arg)>;
        std::vector<std::pair<std::string, std::string>> pairs;
        if constexpr (std::is_same_v<T, CustomEthWalletOptions>) {
            pairs.emplace_back("type", KosNitroJson::string("CustomEth"));
            pairs.emplace_back("chain_id", KosNitroJson::number(arg.chain_id));
        } else if constexpr (std::is_same_v<T, CustomIcpWalletOptions>) {
            pairs.emplace_back("type", KosNitroJson::string("CustomIcp"));
            pairs.emplace_back("key_type", KosNitroJson::string(arg.key_type));
        }
        return KosNitroJson::object(pairs);
    }, specific);
}

std::string HybridKosNitro::buildWalletOptionsJson(const WalletOptions& options) {
    std::vector<std::pair<std::string, std::string>> pairs;
    pairs.emplace_back("use_legacy_path", KosNitroJson::boolean(options.use_legacy_path));
    if (options.specific.has_value()) {
        pairs.emplace_back("specific", buildWalletChainOptionsJson(options.specific.value()));
    }
    return KosNitroJson::object(pairs);
}

std::string HybridKosNitro::buildAccountJson(const KOSAccount& account) {
    std::vector<std::pair<std::string, std::string>> pairs;
    pairs.emplace_back("chain_id", KosNitroJson::number(account.chain_id));
    pairs.emplace_back("private_key", KosNitroJson::string(account.private_key));
    pairs.emplace_back("public_key", KosNitroJson::string(account.public_key));
    pairs.emplace_back("address", KosNitroJson::string(account.address));
    pairs.emplace_back("path", KosNitroJson::string(account.path));
    if (account.options.has_value()) {
        pairs.emplace_back("options", buildWalletOptionsJson(account.options.value()));
    }
    return KosNitroJson::object(pairs);
}

std::string HybridKosNitro::buildTransactionOptionsJson(const std::variant<EvmTransactionOptions, BtcTransactionOptions, SubstrateTransactionOptions, CosmosTransactionOptions>& options) {
    return std::visit([](auto&& arg) -> std::string {
        using T = std::decay_t<decltype(arg)>;
        std::vector<std::pair<std::string, std::string>> pairs;
        if constexpr (std::is_same_v<T, EvmTransactionOptions>) {
            pairs.emplace_back("type", KosNitroJson::string("Evm"));
            pairs.emplace_back("chain_id", KosNitroJson::number(arg.chain_id));
        } else if constexpr (std::is_same_v<T, BtcTransactionOptions>) {
            pairs.emplace_back("type", KosNitroJson::string("Btc"));
            pairs.emplace_back("prev_scripts", KosNitroJson::stringArray(arg.prev_scripts));
            pairs.emplace_back("input_amounts", KosNitroJson::numberArray(arg.input_amounts));
        } else if constexpr (std::is_same_v<T, SubstrateTransactionOptions>) {
            pairs.emplace_back("type", KosNitroJson::string("Substrate"));
            pairs.emplace_back("call", KosNitroJson::string(arg.call));
            pairs.emplace_back("era", KosNitroJson::string(arg.era));
            pairs.emplace_back("nonce", KosNitroJson::number(arg.nonce));
            pairs.emplace_back("tip", KosNitroJson::number(arg.tip));
            pairs.emplace_back("asset_id", arg.asset_id.has_value() ? KosNitroJson::string(arg.asset_id.value()) : KosNitroJson::null());
            pairs.emplace_back("block_hash", KosNitroJson::string(arg.block_hash));
            pairs.emplace_back("genesis_hash", KosNitroJson::string(arg.genesis_hash));
            pairs.emplace_back("spec_version", KosNitroJson::number(arg.spec_version));
            pairs.emplace_back("transaction_version", KosNitroJson::number(arg.transaction_version));
            pairs.emplace_back("app_id", arg.app_id.has_value() ? KosNitroJson::number(arg.app_id.value()) : KosNitroJson::null());
            pairs.emplace_back("signed_extensions", arg.signed_extensions.has_value() ? KosNitroJson::stringArray(arg.signed_extensions.value()) : KosNitroJson::null());
        } else if constexpr (std::is_same_v<T, CosmosTransactionOptions>) {
            pairs.emplace_back("type", KosNitroJson::string("Cosmos"));
            pairs.emplace_back("chain_id", KosNitroJson::string(arg.chain_id));
            pairs.emplace_back("account_number", KosNitroJson::number(arg.account_number));
        }
        return KosNitroJson::object(pairs);
    }, options);
}

std::string HybridKosNitro::buildBigNumberJson(const BigNumber& value) {
    return KosNitroJson::object({{"value", KosNitroJson::string(value.value)}});
}

// ---------------------------------------------------------------------------
// JSON parsers
// ---------------------------------------------------------------------------

std::variant<CustomEthWalletOptions, CustomIcpWalletOptions> HybridKosNitro::parseWalletChainOptionsJson(const std::string& json) {
    std::string type = KosNitroJson::getString(json, "type");
    if (type == "CustomEth") {
        return CustomEthWalletOptions(WalletChainOptionType::CUSTOMETH, KosNitroJson::getNumber(json, "chain_id"));
    } else if (type == "CustomIcp") {
        return CustomIcpWalletOptions(WalletChainOptionType::CUSTOMICP, KosNitroJson::getString(json, "key_type"));
    }
    throw std::runtime_error("Unknown wallet chain option type: " + type);
}

WalletOptions HybridKosNitro::parseWalletOptionsJson(const std::string& json) {
    bool use_legacy_path = KosNitroJson::getBool(json, "use_legacy_path");
    std::optional<std::variant<CustomEthWalletOptions, CustomIcpWalletOptions>> specific = std::nullopt;
    if (KosNitroJson::hasKey(json, "specific")) {
        std::string_view obj = KosNitroJson::getObject(json, "specific");
        if (!obj.empty()) {
            specific = parseWalletChainOptionsJson(std::string(obj));
        }
    }
    return WalletOptions(use_legacy_path, specific);
}

KOSAccount HybridKosNitro::parseAccountJson(const std::string& json) {
    double chain_id = KosNitroJson::getNumber(json, "chain_id");
    std::string private_key = KosNitroJson::getString(json, "private_key");
    std::string public_key = KosNitroJson::getString(json, "public_key");
    std::string address = KosNitroJson::getString(json, "address");
    std::string path = KosNitroJson::getString(json, "path");
    std::optional<WalletOptions> options = std::nullopt;
    if (KosNitroJson::hasKey(json, "options")) {
        std::string_view obj = KosNitroJson::getObject(json, "options");
        if (!obj.empty()) {
            options = parseWalletOptionsJson(std::string(obj));
        }
    }
    return KOSAccount(chain_id, private_key, public_key, address, path, options);
}

KOSTransaction HybridKosNitro::parseTransactionJson(const std::string& json) {
    double chain_id = KosNitroJson::getNumber(json, "chain_id");
    std::string raw = KosNitroJson::getString(json, "raw");
    std::string sender = KosNitroJson::getString(json, "sender");
    std::string signature = KosNitroJson::getString(json, "signature");
    return KOSTransaction(chain_id, raw, sender, signature);
}

// ---------------------------------------------------------------------------
// Public API
// ---------------------------------------------------------------------------

std::string HybridKosNitro::generateMnemonic(double size) {
    return callString(kos_nitro_generate_mnemonic(static_cast<int32_t>(size)));
}

bool HybridKosNitro::validateMnemonic(const std::string& mnemonic) {
    bool result = kos_nitro_validate_mnemonic(mnemonic.c_str()) != 0;
    std::string err = getLastError();
    if (!err.empty()) throw std::runtime_error("kos-nitro error: " + err);
    return result;
}

std::string HybridKosNitro::getPathByChain(double chainId, double index, bool useLegacyPath) {
    return callString(kos_nitro_get_path_by_chain(static_cast<uint32_t>(chainId), static_cast<uint32_t>(index), useLegacyPath ? 1 : 0));
}

KOSAccount HybridKosNitro::generateWalletFromMnemonic(const std::string& mnemonic, double chainId, double index, const std::optional<WalletOptions>& options) {
    const char* optionsPtr = nullptr;
    std::string optionsJson;
    if (options.has_value()) {
        optionsJson = buildWalletOptionsJson(options.value());
        optionsPtr = optionsJson.c_str();
    }
    std::string result = callString(kos_nitro_generate_wallet_from_mnemonic(mnemonic.c_str(), static_cast<uint32_t>(chainId), static_cast<uint32_t>(index), optionsPtr));
    return parseAccountJson(result);
}

KOSAccount HybridKosNitro::generateWalletFromPrivateKey(double chainId, const std::string& privateKey, const std::optional<WalletOptions>& options) {
    const char* optionsPtr = nullptr;
    std::string optionsJson;
    if (options.has_value()) {
        optionsJson = buildWalletOptionsJson(options.value());
        optionsPtr = optionsJson.c_str();
    }
    std::string result = callString(kos_nitro_generate_wallet_from_private_key(static_cast<uint32_t>(chainId), privateKey.c_str(), optionsPtr));
    return parseAccountJson(result);
}

KOSTransaction HybridKosNitro::signTransaction(const KOSAccount& account, const std::string& raw, const std::optional<std::variant<EvmTransactionOptions, BtcTransactionOptions, SubstrateTransactionOptions, CosmosTransactionOptions>>& options) {
    std::string accountJson = buildAccountJson(account);
    const char* optionsPtr = nullptr;
    std::string optionsJson;
    if (options.has_value()) {
        optionsJson = buildTransactionOptionsJson(options.value());
        optionsPtr = optionsJson.c_str();
    }
    std::string result = callString(kos_nitro_sign_transaction(accountJson.c_str(), raw.c_str(), optionsPtr));
    return parseTransactionJson(result);
}

std::shared_ptr<ArrayBuffer> HybridKosNitro::signMessage(const KOSAccount& account, const std::string& hex, bool legacy) {
    std::string accountJson = buildAccountJson(account);
    std::string result = callString(kos_nitro_sign_message(accountJson.c_str(), hex.c_str(), legacy ? 1 : 0));
    return hexToArrayBuffer(result);
}

bool HybridKosNitro::isChainSupported(double chainId) {
    bool result = kos_nitro_is_chain_supported(static_cast<uint32_t>(chainId)) != 0;
    std::string err = getLastError();
    if (!err.empty()) throw std::runtime_error("kos-nitro error: " + err);
    return result;
}

std::vector<double> HybridKosNitro::getSupportedChains() {
    std::string result = callString(kos_nitro_get_supported_chains());
    return KosNitroJson::parseNumberArray(result);
}

std::string HybridKosNitro::encryptWithGcm(const std::string& data, const std::string& password, double iterations) {
    return callString(kos_nitro_encrypt_with_gcm(data.c_str(), password.c_str(), static_cast<uint32_t>(iterations)));
}

std::string HybridKosNitro::encryptWithCbc(const std::string& data, const std::string& password, double iterations) {
    return callString(kos_nitro_encrypt_with_cbc(data.c_str(), password.c_str(), static_cast<uint32_t>(iterations)));
}

std::string HybridKosNitro::encryptWithCfb(const std::string& data, const std::string& password, double iterations) {
    return callString(kos_nitro_encrypt_with_cfb(data.c_str(), password.c_str(), static_cast<uint32_t>(iterations)));
}

std::string HybridKosNitro::decrypt(const std::string& data, const std::string& password, double iterations) {
    return callString(kos_nitro_decrypt(data.c_str(), password.c_str(), static_cast<uint32_t>(iterations)));
}

BigNumber HybridKosNitro::bigNumberNew(const std::string& value) {
    std::string result = callString(kos_nitro_big_number_new(buildBigNumberJson(BigNumber(value)).c_str()));
    return BigNumber(KosNitroJson::getString(result, "value"));
}

std::string HybridKosNitro::bigNumberString(const BigNumber& value) {
    return callString(kos_nitro_big_number_string(buildBigNumberJson(value).c_str()));
}

BigNumber HybridKosNitro::bigNumberAdd(const BigNumber& lhs, const BigNumber& rhs) {
    std::string result = callString(kos_nitro_big_number_add(buildBigNumberJson(lhs).c_str(), buildBigNumberJson(rhs).c_str()));
    return BigNumber(KosNitroJson::getString(result, "value"));
}

BigNumber HybridKosNitro::bigNumberSubtract(const BigNumber& lhs, const BigNumber& rhs) {
    std::string result = callString(kos_nitro_big_number_subtract(buildBigNumberJson(lhs).c_str(), buildBigNumberJson(rhs).c_str()));
    return BigNumber(KosNitroJson::getString(result, "value"));
}

BigNumber HybridKosNitro::bigNumberMultiply(const BigNumber& lhs, const BigNumber& rhs) {
    std::string result = callString(kos_nitro_big_number_multiply(buildBigNumberJson(lhs).c_str(), buildBigNumberJson(rhs).c_str()));
    return BigNumber(KosNitroJson::getString(result, "value"));
}

BigNumber HybridKosNitro::bigNumberDivide(const BigNumber& lhs, const BigNumber& rhs) {
    std::string result = callString(kos_nitro_big_number_divide(buildBigNumberJson(lhs).c_str(), buildBigNumberJson(rhs).c_str()));
    return BigNumber(KosNitroJson::getString(result, "value"));
}

BigNumber HybridKosNitro::bigNumberPow(const BigNumber& base, const BigNumber& exponent) {
    std::string result = callString(kos_nitro_big_number_pow(buildBigNumberJson(base).c_str(), buildBigNumberJson(exponent).c_str()));
    return BigNumber(KosNitroJson::getString(result, "value"));
}

bool HybridKosNitro::bigNumberIsEqual(const BigNumber& lhs, const BigNumber& rhs) {
    bool result = kos_nitro_big_number_is_equal(buildBigNumberJson(lhs).c_str(), buildBigNumberJson(rhs).c_str()) != 0;
    std::string err = getLastError();
    if (!err.empty()) throw std::runtime_error("kos-nitro error: " + err);
    return result;
}

bool HybridKosNitro::bigNumberIsGt(const BigNumber& lhs, const BigNumber& rhs) {
    bool result = kos_nitro_big_number_is_gt(buildBigNumberJson(lhs).c_str(), buildBigNumberJson(rhs).c_str()) != 0;
    std::string err = getLastError();
    if (!err.empty()) throw std::runtime_error("kos-nitro error: " + err);
    return result;
}

bool HybridKosNitro::bigNumberIsGte(const BigNumber& lhs, const BigNumber& rhs) {
    bool result = kos_nitro_big_number_is_gte(buildBigNumberJson(lhs).c_str(), buildBigNumberJson(rhs).c_str()) != 0;
    std::string err = getLastError();
    if (!err.empty()) throw std::runtime_error("kos-nitro error: " + err);
    return result;
}

bool HybridKosNitro::bigNumberIsLt(const BigNumber& lhs, const BigNumber& rhs) {
    bool result = kos_nitro_big_number_is_lt(buildBigNumberJson(lhs).c_str(), buildBigNumberJson(rhs).c_str()) != 0;
    std::string err = getLastError();
    if (!err.empty()) throw std::runtime_error("kos-nitro error: " + err);
    return result;
}

bool HybridKosNitro::bigNumberIsLte(const BigNumber& lhs, const BigNumber& rhs) {
    bool result = kos_nitro_big_number_is_lte(buildBigNumberJson(lhs).c_str(), buildBigNumberJson(rhs).c_str()) != 0;
    std::string err = getLastError();
    if (!err.empty()) throw std::runtime_error("kos-nitro error: " + err);
    return result;
}

bool HybridKosNitro::bigNumberIsZero(const BigNumber& value) {
    bool result = kos_nitro_big_number_is_zero(buildBigNumberJson(value).c_str()) != 0;
    std::string err = getLastError();
    if (!err.empty()) throw std::runtime_error("kos-nitro error: " + err);
    return result;
}

bool HybridKosNitro::bigNumberIsPositive(const BigNumber& value) {
    bool result = kos_nitro_big_number_is_positive(buildBigNumberJson(value).c_str()) != 0;
    std::string err = getLastError();
    if (!err.empty()) throw std::runtime_error("kos-nitro error: " + err);
    return result;
}

bool HybridKosNitro::bigNumberIsNegative(const BigNumber& value) {
    bool result = kos_nitro_big_number_is_negative(buildBigNumberJson(value).c_str()) != 0;
    std::string err = getLastError();
    if (!err.empty()) throw std::runtime_error("kos-nitro error: " + err);
    return result;
}

std::shared_ptr<ArrayBuffer> HybridKosNitro::generateXpub(const std::string& mnemonic, const std::string& passphrase, bool isMainnet, double index) {
    std::string result = callString(kos_nitro_generate_xpub(mnemonic.c_str(), passphrase.c_str(), isMainnet ? 1 : 0, static_cast<uint32_t>(index)));
    return hexToArrayBuffer(result);
}

std::string HybridKosNitro::getXpubAsString(const std::string& mnemonic, const std::string& passphrase, bool isMainnet, double index) {
    return callString(kos_nitro_get_xpub_as_string(mnemonic.c_str(), passphrase.c_str(), isMainnet ? 1 : 0, static_cast<uint32_t>(index)));
}

std::shared_ptr<ArrayBuffer> HybridKosNitro::deriveXpub(const std::string& mnemonic, const std::string& passphrase, bool isMainnet, double index, const std::string& derivationPath) {
    std::string result = callString(kos_nitro_derive_xpub(mnemonic.c_str(), passphrase.c_str(), isMainnet ? 1 : 0, static_cast<uint32_t>(index), derivationPath.c_str()));
    return hexToArrayBuffer(result);
}

std::shared_ptr<ArrayBuffer> HybridKosNitro::slip77MasterBlindingKey(const std::string& mnemonic, const std::string& passphrase, bool isMainnet, double index) {
    std::string result = callString(kos_nitro_slip77_master_blinding_key(mnemonic.c_str(), passphrase.c_str(), isMainnet ? 1 : 0, static_cast<uint32_t>(index)));
    return hexToArrayBuffer(result);
}

std::shared_ptr<ArrayBuffer> HybridKosNitro::signEcdsaRecoverable(const std::string& mnemonic, const std::string& passphrase, bool isMainnet, double index, const std::string& msgHex) {
    std::string result = callString(kos_nitro_sign_ecdsa_recoverable(mnemonic.c_str(), passphrase.c_str(), isMainnet ? 1 : 0, static_cast<uint32_t>(index), msgHex.c_str()));
    return hexToArrayBuffer(result);
}

std::shared_ptr<ArrayBuffer> HybridKosNitro::hmacSha256(const std::string& mnemonic, const std::string& passphrase, bool isMainnet, double index, const std::string& derivationPath, const std::string& msgHex) {
    std::string result = callString(kos_nitro_hmac_sha256(mnemonic.c_str(), passphrase.c_str(), isMainnet ? 1 : 0, static_cast<uint32_t>(index), derivationPath.c_str(), msgHex.c_str()));
    return hexToArrayBuffer(result);
}

std::shared_ptr<ArrayBuffer> HybridKosNitro::eciesEncrypt(const std::string& mnemonic, const std::string& passphrase, bool isMainnet, double index, const std::string& msgHex) {
    std::string result = callString(kos_nitro_ecies_encrypt(mnemonic.c_str(), passphrase.c_str(), isMainnet ? 1 : 0, static_cast<uint32_t>(index), msgHex.c_str()));
    return hexToArrayBuffer(result);
}

std::shared_ptr<ArrayBuffer> HybridKosNitro::eciesDecrypt(const std::string& mnemonic, const std::string& passphrase, bool isMainnet, double index, const std::string& msgHex) {
    std::string result = callString(kos_nitro_ecies_decrypt(mnemonic.c_str(), passphrase.c_str(), isMainnet ? 1 : 0, static_cast<uint32_t>(index), msgHex.c_str()));
    return hexToArrayBuffer(result);
}

} // namespace margelo::nitro::kosnitro
