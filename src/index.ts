import { NitroModules } from 'react-native-nitro-modules'
import type { KosNitro as KosNitroSpec } from './specs/kos-nitro.nitro'

export const KosNitro =
  NitroModules.createHybridObject<KosNitroSpec>('KosNitro')

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