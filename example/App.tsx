import { useState, useEffect } from 'react';
import {
  Button,
  ScrollView,
  StyleSheet,
  Text,
  View,
} from 'react-native';
import { KosNitro } from 'react-native-kos-nitro';

export default function App() {
  const [mnemonic, setMnemonic] = useState<string>('');
  const [account, setAccount] = useState<string>('');
  const [signature, setSignature] = useState<string>('');
  const [supportedChains, setSupportedChains] = useState<number[]>([]);
  const [error, setError] = useState<string>('');

  useEffect(() => {
    try {
      const chains = KosNitro.getSupportedChains();
      setSupportedChains(chains);
    } catch (e) {
      setError(String(e));
    }
  }, []);

  const generateMnemonic = () => {
    try {
      setError('');
      const m = KosNitro.generateMnemonic(24);
      setMnemonic(m);
    } catch (e) {
      setError(String(e));
    }
  };

  const generateWallet = () => {
    if (!mnemonic) return;
    try {
      setError('');
      const acc = KosNitro.generateWalletFromMnemonic(mnemonic, 38, 0);
      setAccount(JSON.stringify(acc, null, 2));
    } catch (e) {
      setError(String(e));
    }
  };

  const signMessage = () => {
    if (!account) return;
    try {
      setError('');
      const acc = JSON.parse(account);
      const sig = KosNitro.signMessage(acc, 'deadbeef', false);
      setSignature(
        Array.from(new Uint8Array(sig))
          .map((b) => b.toString(16).padStart(2, '0'))
          .join('')
      );
    } catch (e) {
      setError(String(e));
    }
  };

  return (
    <ScrollView contentContainerStyle={styles.container}>
      <Text style={styles.title}>KosNitro Example</Text>

      <Text style={styles.label}>Supported chains:</Text>
      <Text style={styles.value}>{supportedChains.join(', ')}</Text>

      <Button title="Generate Mnemonic (24 words)" onPress={generateMnemonic} />
      {mnemonic ? (
        <>
          <Text style={styles.label}>Mnemonic:</Text>
          <Text style={styles.value}>{mnemonic}</Text>
        </>
      ) : null}

      <Button title="Generate Wallet (KLV)" onPress={generateWallet} />
      {account ? (
        <>
          <Text style={styles.label}>Account:</Text>
          <Text style={styles.value}>{account}</Text>
        </>
      ) : null}

      <Button title="Sign Message" onPress={signMessage} />
      {signature ? (
        <>
          <Text style={styles.label}>Signature:</Text>
          <Text style={styles.value}>{signature}</Text>
        </>
      ) : null}

      {error ? <Text style={styles.error}>Error: {error}</Text> : null}
    </ScrollView>
  );
}

const styles = StyleSheet.create({
  container: {
    padding: 20,
    paddingTop: 60,
    gap: 12,
  },
  title: {
    fontSize: 24,
    fontWeight: 'bold',
    marginBottom: 12,
  },
  label: {
    fontWeight: '600',
    marginTop: 8,
  },
  value: {
    fontSize: 12,
    fontFamily: 'monospace',
    backgroundColor: '#f0f0f0',
    padding: 8,
    borderRadius: 4,
  },
  error: {
    color: 'red',
    marginTop: 12,
  },
});