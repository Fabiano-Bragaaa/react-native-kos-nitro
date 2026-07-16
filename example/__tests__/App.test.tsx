/**
 * @format
 */

import React from 'react';
import ReactTestRenderer from 'react-test-renderer';
import App from '../App';

jest.mock('react-native-kos-nitro', () => ({
  KosNitro: {
    generateMnemonic: jest.fn(() => 'test test test test test test test test test test test test'),
    validateMnemonic: jest.fn(() => true),
    generateWalletFromMnemonic: jest.fn(() => ({
      chain_id: 38,
      private_key: 'privkey',
      public_key: 'pubkey',
      address: 'klv1test',
      path: "m/44'/691'/0'/0/0",
    })),
    signMessage: jest.fn(() => new ArrayBuffer(64)),
    isChainSupported: jest.fn(() => true),
    getSupportedChains: jest.fn(() => [38]),
    encryptWithGcm: jest.fn((data: string) => data),
    decrypt: jest.fn((data: string) => data),
    bigNumberNew: jest.fn((value: string) => ({ value })),
    bigNumberAdd: jest.fn((lhs: any, rhs: any) => ({
      value: String(Number(lhs.value) + Number(rhs.value)),
    })),
    generateXpub: jest.fn(() => new ArrayBuffer(78)),
  },
}));

test('renders correctly', async () => {
  await ReactTestRenderer.act(() => {
    ReactTestRenderer.create(<App />);
  });
});
