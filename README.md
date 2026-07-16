# react-native-kos-nitro

React Native bindings for the [KOS](https://github.com/klever-io/kos-rs) wallet
core (Klever), exposed through [Nitro modules](https://nitro.margelo.com). It
runs the Rust `kos` crates natively via a C-ABI, giving you mnemonic
generation, HD key derivation, transaction/message signing, encryption and
big-number math for many chains — synchronously, off the JS bridge.

[![Version](https://img.shields.io/npm/v/react-native-kos-nitro.svg)](https://www.npmjs.com/package/react-native-kos-nitro)

## Requirements

- React Native 0.76.0 or higher (New Architecture — Fabric/TurboModules)
- Node 18 or higher

## Installation

```sh
pnpm add react-native-kos-nitro react-native-nitro-modules
# or: npm i / yarn add
```

`react-native-nitro-modules` is a required peer dependency.

### iOS

```sh
cd ios && pod install
```

### Android

No extra steps — the prebuilt native library is linked automatically by the
package's Gradle/CMake config.

## Prebuilt binaries

The native code is written in Rust and shipped as **prebuilt static libraries**
(an iOS `xcframework` and Android `jniLibs`). To keep the npm package small,
those binaries are **not** in the tarball — a `postinstall` script
(`scripts/fetch-binaries.js`) downloads them from the GitHub Release whose tag
matches the installed version and extracts them in place.

Environment variables:

| Variable | Effect |
| --- | --- |
| `KOS_NITRO_SKIP_DOWNLOAD=1` | Skip the download (you must then provide the binaries yourself, e.g. an offline/CI cache). |
| `KOS_NITRO_PLATFORMS=ios` | Download only the listed platforms (`ios`, `android`, or both — default). |

The download is idempotent: it is skipped when the binaries are already present.

## Usage

Import the functions you need directly:

```ts
import {
  getSupportedChains,
  generateMnemonic,
  generateWalletFromMnemonic,
  signMessage,
} from 'react-native-kos-nitro'

// Which chains are supported (numeric chain ids)
const chains = getSupportedChains()

// Generate a 24-word mnemonic
const mnemonic = generateMnemonic(24)

// Derive an account — chain 38 = KLV, index 0
const account = generateWalletFromMnemonic(mnemonic, 38, 0)
// account: { chain_id, private_key, public_key, address, path, options? }

// Sign a message (hex payload); returns an ArrayBuffer
const sig = signMessage(account, 'deadbeef', false)
const sigHex = Array.from(new Uint8Array(sig))
  .map((b) => b.toString(16).padStart(2, '0'))
  .join('')
```

Prefer the named imports above. The whole native instance is also exported as
`KosNitro` (e.g. `import { KosNitro } from 'react-native-kos-nitro'` then
`KosNitro.generateMnemonic(24)`) if you'd rather have a single object.

All methods are **synchronous** and run on the calling thread. For heavy calls,
invoke them from a worklet/background thread if you need to keep the UI thread
free.

## API

### Wallet & keys

- `generateMnemonic(size: number): string`
- `validateMnemonic(mnemonic: string): boolean`
- `getPathByChain(chainId, index, useLegacyPath): string`
- `generateWalletFromMnemonic(mnemonic, chainId, index, options?): KOSAccount`
- `generateWalletFromPrivateKey(chainId, privateKey, options?): KOSAccount`

### Signing

- `signTransaction(account, raw, options?): KOSTransaction`
- `signMessage(account, hex, legacy): ArrayBuffer`
- `signEcdsaRecoverable(mnemonic, passphrase, isMainnet, index, msgHex): ArrayBuffer`

### Chains

- `isChainSupported(chainId): boolean`
- `getSupportedChains(): number[]`

### Encryption

- `encryptWithGcm(data, password, iterations): string`
- `encryptWithCbc(data, password, iterations): string`
- `encryptWithCfb(data, password, iterations): string`
- `decrypt(data, password, iterations): string`
- `eciesEncrypt(...) / eciesDecrypt(...): ArrayBuffer`

### Extended public keys (Liquid / BIP32)

- `generateXpub(...) / getXpubAsString(...) / deriveXpub(...)`
- `slip77MasterBlindingKey(...)`
- `hmacSha256(...)`

### BigNumber

`bigNumberNew`, `bigNumberString`, `bigNumberAdd`, `bigNumberSubtract`,
`bigNumberMultiply`, `bigNumberDivide`, `bigNumberPow`, and comparison helpers
(`bigNumberIsEqual`, `bigNumberIsGt/Gte/Lt/Lte`, `bigNumberIsZero`,
`bigNumberIsPositive`, `bigNumberIsNegative`).

See [`src/specs/kos-nitro.nitro.ts`](src/specs/kos-nitro.nitro.ts) for the full
typed surface (`KOSAccount`, `KOSTransaction`, `WalletOptions`,
`TransactionChainOptions`, etc.).

## Building the binaries from source

You only need this if you're contributing or want to rebuild the native libs.
Requires the Rust toolchain, `cargo-ndk` (Android) and Xcode (iOS).

```sh
./build_ios.sh       # -> ios/KosNitro.xcframework
./build_android.sh   # -> android/src/main/jniLibs/<abi>/libkos_nitro.a
```

The Rust crates (`kos`, `kos-codec`, `kos-mobile`) are vendored under
`rust/vendor/` and the FFI wrapper lives in `rust/kos-nitro`.

## Example

A runnable example app lives in [`example/`](example/):

```sh
pnpm install
cd example
pnpm ios     # or: pnpm android
```

## License & attribution

The binding code in this repository is MIT licensed.

The vendored Rust crates under `rust/vendor/` (`kos`, `kos-codec`, `kos-mobile`)
are part of [klever-io/kos-rs](https://github.com/klever-io/kos-rs) and are
licensed under Apache-2.0 — © Klever. The original React Native / Nitro binding
work is by [Gustavo Santos (ghsantos)](https://github.com/ghsantos).
