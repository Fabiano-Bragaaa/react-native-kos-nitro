import { NitroModules } from 'react-native-nitro-modules'
import type { KosNitro as KosNitroSpec } from './specs/kos-nitro.nitro'

const instance = NitroModules.createHybridObject<KosNitroSpec>('KosNitro')

/**
 * The native KosNitro HybridObject. Kept for backwards compatibility and for
 * cases where you want the whole instance; prefer the named function exports
 * below (`import { generateMnemonic } from 'react-native-kos-nitro'`).
 */
export const KosNitro = instance

// Wallet & keys
export const generateMnemonic = instance.generateMnemonic.bind(instance)
export const validateMnemonic = instance.validateMnemonic.bind(instance)
export const getPathByChain = instance.getPathByChain.bind(instance)
export const generateWalletFromMnemonic =
  instance.generateWalletFromMnemonic.bind(instance)
export const generateWalletFromPrivateKey =
  instance.generateWalletFromPrivateKey.bind(instance)

// Signing
export const signTransaction = instance.signTransaction.bind(instance)
export const signMessage = instance.signMessage.bind(instance)
export const signEcdsaRecoverable =
  instance.signEcdsaRecoverable.bind(instance)

// Chains
export const isChainSupported = instance.isChainSupported.bind(instance)
export const getSupportedChains = instance.getSupportedChains.bind(instance)

// Encryption
export const encryptWithGcm = instance.encryptWithGcm.bind(instance)
export const encryptWithCbc = instance.encryptWithCbc.bind(instance)
export const encryptWithCfb = instance.encryptWithCfb.bind(instance)
export const decrypt = instance.decrypt.bind(instance)
export const eciesEncrypt = instance.eciesEncrypt.bind(instance)
export const eciesDecrypt = instance.eciesDecrypt.bind(instance)

// Extended public keys (Liquid / BIP32)
export const generateXpub = instance.generateXpub.bind(instance)
export const getXpubAsString = instance.getXpubAsString.bind(instance)
export const deriveXpub = instance.deriveXpub.bind(instance)
export const slip77MasterBlindingKey =
  instance.slip77MasterBlindingKey.bind(instance)
export const hmacSha256 = instance.hmacSha256.bind(instance)

// BigNumber
export const bigNumberNew = instance.bigNumberNew.bind(instance)
export const bigNumberString = instance.bigNumberString.bind(instance)
export const bigNumberAdd = instance.bigNumberAdd.bind(instance)
export const bigNumberSubtract = instance.bigNumberSubtract.bind(instance)
export const bigNumberMultiply = instance.bigNumberMultiply.bind(instance)
export const bigNumberDivide = instance.bigNumberDivide.bind(instance)
export const bigNumberPow = instance.bigNumberPow.bind(instance)
export const bigNumberIsEqual = instance.bigNumberIsEqual.bind(instance)
export const bigNumberIsGt = instance.bigNumberIsGt.bind(instance)
export const bigNumberIsGte = instance.bigNumberIsGte.bind(instance)
export const bigNumberIsLt = instance.bigNumberIsLt.bind(instance)
export const bigNumberIsLte = instance.bigNumberIsLte.bind(instance)
export const bigNumberIsZero = instance.bigNumberIsZero.bind(instance)
export const bigNumberIsPositive =
  instance.bigNumberIsPositive.bind(instance)
export const bigNumberIsNegative =
  instance.bigNumberIsNegative.bind(instance)

export type {
  KosNitro as KosNitroType,
  KOSAccount,
  KOSTransaction,
  WalletOptions,
  WalletChainOptions,
  WalletChainOptionType,
  CustomEthWalletOptions,
  CustomIcpWalletOptions,
  TransactionChainOptions,
  TransactionChainOptionType,
  EvmTransactionOptions,
  BtcTransactionOptions,
  SubstrateTransactionOptions,
  CosmosTransactionOptions,
  BigNumber,
} from './specs/kos-nitro.nitro'
