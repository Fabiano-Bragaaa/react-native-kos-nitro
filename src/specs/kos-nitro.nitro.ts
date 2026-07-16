import type { HybridObject, UInt64 } from 'react-native-nitro-modules'

export enum WalletChainOptionType {
  CustomEth = 0,
  CustomIcp = 1,
}

export interface CustomEthWalletOptions {
  type: WalletChainOptionType.CustomEth
  chain_id: number
}

export interface CustomIcpWalletOptions {
  type: WalletChainOptionType.CustomIcp
  key_type: string
}

export type WalletChainOptions = CustomEthWalletOptions | CustomIcpWalletOptions

export interface WalletOptions {
  use_legacy_path: boolean
  specific?: WalletChainOptions
}

export interface KOSAccount {
  chain_id: number
  private_key: string
  public_key: string
  address: string
  path: string
  options?: WalletOptions
}

export interface KOSTransaction {
  chain_id: number
  raw: string
  sender: string
  signature: string
}

export enum TransactionChainOptionType {
  Evm = 0,
  Btc = 1,
  Substrate = 2,
  Cosmos = 3,
}

export interface EvmTransactionOptions {
  type: TransactionChainOptionType.Evm
  chain_id: number
}

export interface BtcTransactionOptions {
  type: TransactionChainOptionType.Btc
  prev_scripts: string[]
  input_amounts: UInt64[]
}

export interface SubstrateTransactionOptions {
  type: TransactionChainOptionType.Substrate
  call: string
  era: string
  nonce: number
  tip: UInt64
  asset_id?: string
  block_hash: string
  genesis_hash: string
  spec_version: number
  transaction_version: number
  app_id?: number
  signed_extensions?: string[]
}

export interface CosmosTransactionOptions {
  type: TransactionChainOptionType.Cosmos
  chain_id: string
  account_number: UInt64
}

export type TransactionChainOptions =
  | EvmTransactionOptions
  | BtcTransactionOptions
  | SubstrateTransactionOptions
  | CosmosTransactionOptions

export interface BigNumber {
  value: string
}

export interface KosNitro extends HybridObject<{ ios: 'c++', android: 'c++' }> {
  generateMnemonic(size: number): string
  validateMnemonic(mnemonic: string): boolean
  getPathByChain(chainId: number, index: number, useLegacyPath: boolean): string
  generateWalletFromMnemonic(
    mnemonic: string,
    chainId: number,
    index: number,
    options?: WalletOptions
  ): KOSAccount
  generateWalletFromPrivateKey(
    chainId: number,
    privateKey: string,
    options?: WalletOptions
  ): KOSAccount
  signTransaction(
    account: KOSAccount,
    raw: string,
    options?: TransactionChainOptions
  ): KOSTransaction
  signMessage(account: KOSAccount, hex: string, legacy: boolean): ArrayBuffer
  isChainSupported(chainId: number): boolean
  getSupportedChains(): number[]
  encryptWithGcm(data: string, password: string, iterations: number): string
  encryptWithCbc(data: string, password: string, iterations: number): string
  encryptWithCfb(data: string, password: string, iterations: number): string
  decrypt(data: string, password: string, iterations: number): string
  bigNumberNew(value: string): BigNumber
  bigNumberString(value: BigNumber): string
  bigNumberAdd(lhs: BigNumber, rhs: BigNumber): BigNumber
  bigNumberSubtract(lhs: BigNumber, rhs: BigNumber): BigNumber
  bigNumberMultiply(lhs: BigNumber, rhs: BigNumber): BigNumber
  bigNumberDivide(lhs: BigNumber, rhs: BigNumber): BigNumber
  bigNumberPow(base: BigNumber, exponent: BigNumber): BigNumber
  bigNumberIsEqual(lhs: BigNumber, rhs: BigNumber): boolean
  bigNumberIsGt(lhs: BigNumber, rhs: BigNumber): boolean
  bigNumberIsGte(lhs: BigNumber, rhs: BigNumber): boolean
  bigNumberIsLt(lhs: BigNumber, rhs: BigNumber): boolean
  bigNumberIsLte(lhs: BigNumber, rhs: BigNumber): boolean
  bigNumberIsZero(value: BigNumber): boolean
  bigNumberIsPositive(value: BigNumber): boolean
  bigNumberIsNegative(value: BigNumber): boolean
  generateXpub(mnemonic: string, passphrase: string, isMainnet: boolean, index: number): ArrayBuffer
  getXpubAsString(mnemonic: string, passphrase: string, isMainnet: boolean, index: number): string
  deriveXpub(
    mnemonic: string,
    passphrase: string,
    isMainnet: boolean,
    index: number,
    derivationPath: string
  ): ArrayBuffer
  slip77MasterBlindingKey(mnemonic: string, passphrase: string, isMainnet: boolean, index: number): ArrayBuffer
  signEcdsaRecoverable(
    mnemonic: string,
    passphrase: string,
    isMainnet: boolean,
    index: number,
    msgHex: string
  ): ArrayBuffer
  hmacSha256(
    mnemonic: string,
    passphrase: string,
    isMainnet: boolean,
    index: number,
    derivationPath: string,
    msgHex: string
  ): ArrayBuffer
  eciesEncrypt(
    mnemonic: string,
    passphrase: string,
    isMainnet: boolean,
    index: number,
    msgHex: string
  ): ArrayBuffer
  eciesDecrypt(
    mnemonic: string,
    passphrase: string,
    isMainnet: boolean,
    index: number,
    msgHex: string
  ): ArrayBuffer
}