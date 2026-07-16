#pragma once

#include "HybridKosNitroSpec.hpp"

namespace margelo::nitro::kosnitro {

class HybridKosNitro : public HybridKosNitroSpec {
public:
    HybridKosNitro() : HybridObject(TAG), HybridKosNitroSpec() {}

    std::string generateMnemonic(double size) override;
    bool validateMnemonic(const std::string& mnemonic) override;
    std::string getPathByChain(double chainId, double index, bool useLegacyPath) override;
    KOSAccount generateWalletFromMnemonic(const std::string& mnemonic, double chainId, double index, const std::optional<WalletOptions>& options) override;
    KOSAccount generateWalletFromPrivateKey(double chainId, const std::string& privateKey, const std::optional<WalletOptions>& options) override;
    KOSTransaction signTransaction(const KOSAccount& account, const std::string& raw, const std::optional<std::variant<EvmTransactionOptions, BtcTransactionOptions, SubstrateTransactionOptions, CosmosTransactionOptions>>& options) override;
    std::shared_ptr<ArrayBuffer> signMessage(const KOSAccount& account, const std::string& hex, bool legacy) override;
    bool isChainSupported(double chainId) override;
    std::vector<double> getSupportedChains() override;
    std::string encryptWithGcm(const std::string& data, const std::string& password, double iterations) override;
    std::string encryptWithCbc(const std::string& data, const std::string& password, double iterations) override;
    std::string encryptWithCfb(const std::string& data, const std::string& password, double iterations) override;
    std::string decrypt(const std::string& data, const std::string& password, double iterations) override;
    BigNumber bigNumberNew(const std::string& value) override;
    std::string bigNumberString(const BigNumber& value) override;
    BigNumber bigNumberAdd(const BigNumber& lhs, const BigNumber& rhs) override;
    BigNumber bigNumberSubtract(const BigNumber& lhs, const BigNumber& rhs) override;
    BigNumber bigNumberMultiply(const BigNumber& lhs, const BigNumber& rhs) override;
    BigNumber bigNumberDivide(const BigNumber& lhs, const BigNumber& rhs) override;
    BigNumber bigNumberPow(const BigNumber& base, const BigNumber& exponent) override;
    bool bigNumberIsEqual(const BigNumber& lhs, const BigNumber& rhs) override;
    bool bigNumberIsGt(const BigNumber& lhs, const BigNumber& rhs) override;
    bool bigNumberIsGte(const BigNumber& lhs, const BigNumber& rhs) override;
    bool bigNumberIsLt(const BigNumber& lhs, const BigNumber& rhs) override;
    bool bigNumberIsLte(const BigNumber& lhs, const BigNumber& rhs) override;
    bool bigNumberIsZero(const BigNumber& value) override;
    bool bigNumberIsPositive(const BigNumber& value) override;
    bool bigNumberIsNegative(const BigNumber& value) override;
    std::shared_ptr<ArrayBuffer> generateXpub(const std::string& mnemonic, const std::string& passphrase, bool isMainnet, double index) override;
    std::string getXpubAsString(const std::string& mnemonic, const std::string& passphrase, bool isMainnet, double index) override;
    std::shared_ptr<ArrayBuffer> deriveXpub(const std::string& mnemonic, const std::string& passphrase, bool isMainnet, double index, const std::string& derivationPath) override;
    std::shared_ptr<ArrayBuffer> slip77MasterBlindingKey(const std::string& mnemonic, const std::string& passphrase, bool isMainnet, double index) override;
    std::shared_ptr<ArrayBuffer> signEcdsaRecoverable(const std::string& mnemonic, const std::string& passphrase, bool isMainnet, double index, const std::string& msgHex) override;
    std::shared_ptr<ArrayBuffer> hmacSha256(const std::string& mnemonic, const std::string& passphrase, bool isMainnet, double index, const std::string& derivationPath, const std::string& msgHex) override;
    std::shared_ptr<ArrayBuffer> eciesEncrypt(const std::string& mnemonic, const std::string& passphrase, bool isMainnet, double index, const std::string& msgHex) override;
    std::shared_ptr<ArrayBuffer> eciesDecrypt(const std::string& mnemonic, const std::string& passphrase, bool isMainnet, double index, const std::string& msgHex) override;

private:
    std::string callString(char* raw);
    std::string getLastError();
    std::shared_ptr<ArrayBuffer> hexToArrayBuffer(const std::string& hex);
    std::vector<uint8_t> hexToBytes(const std::string& hex);

    std::string buildAccountJson(const KOSAccount& account);
    std::string buildWalletOptionsJson(const WalletOptions& options);
    std::string buildWalletChainOptionsJson(const std::variant<CustomEthWalletOptions, CustomIcpWalletOptions>& specific);
    std::string buildTransactionOptionsJson(const std::variant<EvmTransactionOptions, BtcTransactionOptions, SubstrateTransactionOptions, CosmosTransactionOptions>& options);
    std::string buildBigNumberJson(const BigNumber& value);

    KOSAccount parseAccountJson(const std::string& json);
    WalletOptions parseWalletOptionsJson(const std::string& json);
    std::variant<CustomEthWalletOptions, CustomIcpWalletOptions> parseWalletChainOptionsJson(const std::string& json);
    KOSTransaction parseTransactionJson(const std::string& json);
};

} // namespace margelo::nitro::kosnitro
