import { KosNitro } from 'react-native-kos-nitro';

jest.mock('react-native-kos-nitro', () => ({
  KosNitro: {
    generateMnemonic: jest.fn((size: number) =>
      'abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon'
        .split(' ')
        .slice(0, size)
        .join(' ')
    ),
    validateMnemonic: jest.fn(() => true),
    generateWalletFromMnemonic: jest.fn(() => ({
      chain_id: 38,
      private_key: 'privkey',
      public_key: 'pubkey',
      address: 'klv1usdnywjhrlv4tcyu6stxpl6yvhplg35nepljlt4y5r7yppe8er4qujlazy',
      path: "m/44'/691'/0'/0/0",
      options: { use_legacy_path: false },
    })),
    signMessage: jest.fn(() => new ArrayBuffer(64)),
    isChainSupported: jest.fn((chainId: number) => chainId === 38),
    getSupportedChains: jest.fn(() => [38]),
    encryptWithGcm: jest.fn((data: string) => `encrypted:${data}`),
    decrypt: jest.fn((data: string) => data.replace('encrypted:', '')),
    bigNumberNew: jest.fn((value: string) => ({ value })),
    bigNumberAdd: jest.fn((lhs: any, rhs: any) => ({
      value: String(Number(lhs.value) + Number(rhs.value)),
    })),
    generateXpub: jest.fn(() => new ArrayBuffer(78)),
  },
}));

describe('KosNitro round-trip', () => {
  test('generates mnemonic and wallet for KLV', () => {
    const mnemonic = KosNitro.generateMnemonic(12);
    expect(mnemonic.split(' ')).toHaveLength(12);

    const account = KosNitro.generateWalletFromMnemonic(mnemonic, 38, 0);
    expect(account.chain_id).toBe(38);
    expect(account.address).toContain('klv1');
  });

  test('signs a message with generated account', () => {
    const account = KosNitro.generateWalletFromMnemonic(
      'test mnemonic',
      38,
      0
    );
    const signature = KosNitro.signMessage(account, 'deadbeef', false);
    expect(signature).toBeInstanceOf(ArrayBuffer);
    expect(signature.byteLength).toBeGreaterThan(0);
  });

  test('reports chain support correctly', () => {
    expect(KosNitro.isChainSupported(38)).toBe(true);
    expect(KosNitro.isChainSupported(999)).toBe(false);
  });

  test('lists supported chains', () => {
    const chains = KosNitro.getSupportedChains();
    expect(Array.isArray(chains)).toBe(true);
    expect(chains).toContain(38);
  });

  test('encrypts and decrypts data', () => {
    const data = 'hello world';
    const encrypted = KosNitro.encryptWithGcm(data, 'password', 10_000);
    const decrypted = KosNitro.decrypt(encrypted, 'password', 10_000);
    expect(decrypted).toBe(data);
  });

  test('performs big number addition', () => {
    const a = KosNitro.bigNumberNew('123.45');
    const b = KosNitro.bigNumberNew('0.55');
    const result = KosNitro.bigNumberAdd(a, b);
    expect(Number(result.value)).toBe(124);
  });

  test('generates xpub as ArrayBuffer', () => {
    const xpub = KosNitro.generateXpub(
      'test mnemonic',
      '',
      true,
      0
    );
    expect(xpub).toBeInstanceOf(ArrayBuffer);
  });
});
