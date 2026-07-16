#ifndef KOS_NITRO_H
#define KOS_NITRO_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * All functions that return a `char*` allocate memory on the Rust side and
 * must be freed with `kos_nitro_free_string`. On error they return `nullptr`;
 * call `kos_nitro_last_error` to retrieve a human-readable message.
 */

char* kos_nitro_last_error(void);
void kos_nitro_free_string(char* ptr);

char* kos_nitro_generate_mnemonic(int32_t size);
uint8_t kos_nitro_validate_mnemonic(const char* mnemonic);
char* kos_nitro_get_path_by_chain(uint32_t chain_id, uint32_t index, uint8_t use_legacy_path);
char* kos_nitro_generate_wallet_from_mnemonic(const char* mnemonic,
                                              uint32_t chain_id,
                                              uint32_t index,
                                              const char* options_json);
char* kos_nitro_generate_wallet_from_private_key(uint32_t chain_id,
                                                 const char* private_key,
                                                 const char* options_json);

char* kos_nitro_sign_transaction(const char* account_json,
                                const char* raw,
                                const char* options_json);
char* kos_nitro_sign_message(const char* account_json,
                             const char* message_hex,
                             uint8_t legacy);

uint8_t kos_nitro_is_chain_supported(uint32_t chain_id);
char* kos_nitro_get_supported_chains(void);

char* kos_nitro_encrypt_with_gcm(const char* data,
                                 const char* password,
                                 uint32_t iterations);
char* kos_nitro_encrypt_with_cbc(const char* data,
                                 const char* password,
                                 uint32_t iterations);
char* kos_nitro_encrypt_with_cfb(const char* data,
                                 const char* password,
                                 uint32_t iterations);
char* kos_nitro_decrypt(const char* data,
                        const char* password,
                        uint32_t iterations);

char* kos_nitro_big_number_new(const char* value_json);
char* kos_nitro_big_number_string(const char* value_json);
char* kos_nitro_big_number_add(const char* lhs_json, const char* rhs_json);
char* kos_nitro_big_number_subtract(const char* lhs_json, const char* rhs_json);
char* kos_nitro_big_number_multiply(const char* lhs_json, const char* rhs_json);
char* kos_nitro_big_number_divide(const char* lhs_json, const char* rhs_json);
char* kos_nitro_big_number_pow(const char* base_json, const char* exponent_json);
uint8_t kos_nitro_big_number_is_equal(const char* lhs_json, const char* rhs_json);
uint8_t kos_nitro_big_number_is_gt(const char* lhs_json, const char* rhs_json);
uint8_t kos_nitro_big_number_is_gte(const char* lhs_json, const char* rhs_json);
uint8_t kos_nitro_big_number_is_lt(const char* lhs_json, const char* rhs_json);
uint8_t kos_nitro_big_number_is_lte(const char* lhs_json, const char* rhs_json);
uint8_t kos_nitro_big_number_is_zero(const char* value_json);
uint8_t kos_nitro_big_number_is_positive(const char* value_json);
uint8_t kos_nitro_big_number_is_negative(const char* value_json);

char* kos_nitro_generate_xpub(const char* mnemonic,
                              const char* passphrase,
                              uint8_t is_mainnet,
                              uint32_t index);
char* kos_nitro_get_xpub_as_string(const char* mnemonic,
                                   const char* passphrase,
                                   uint8_t is_mainnet,
                                   uint32_t index);
char* kos_nitro_derive_xpub(const char* mnemonic,
                            const char* passphrase,
                            uint8_t is_mainnet,
                            uint32_t index,
                            const char* derivation_path);
char* kos_nitro_slip77_master_blinding_key(const char* mnemonic,
                                           const char* passphrase,
                                           uint8_t is_mainnet,
                                           uint32_t index);
char* kos_nitro_sign_ecdsa_recoverable(const char* mnemonic,
                                       const char* passphrase,
                                       uint8_t is_mainnet,
                                       uint32_t index,
                                       const char* msg_hex);
char* kos_nitro_hmac_sha256(const char* mnemonic,
                            const char* passphrase,
                            uint8_t is_mainnet,
                            uint32_t index,
                            const char* derivation_path,
                            const char* msg_hex);
char* kos_nitro_ecies_encrypt(const char* mnemonic,
                              const char* passphrase,
                              uint8_t is_mainnet,
                              uint32_t index,
                              const char* msg_hex);
char* kos_nitro_ecies_decrypt(const char* mnemonic,
                              const char* passphrase,
                              uint8_t is_mainnet,
                              uint32_t index,
                              const char* msg_hex);

#ifdef __cplusplus
}
#endif

#endif // KOS_NITRO_H
