#!/usr/bin/env node
'use strict'

// Downloads the prebuilt native binaries (iOS xcframework + Android jniLibs)
// from the GitHub Release whose tag matches this package's version, and
// extracts them in place. The binaries are kept out of the npm tarball and git
// to keep both small (see README "Prebuilt binaries").
//
// Skips work when the target already exists (local dev / re-install) and can be
// disabled entirely with KOS_NITRO_SKIP_DOWNLOAD=1. Select platforms with
// KOS_NITRO_PLATFORMS=ios,android (default: both).

const fs = require('fs')
const path = require('path')
const https = require('https')
const { execFileSync } = require('child_process')
const os = require('os')

const OWNER = 'Fabiano-Bragaaa'
const REPO = 'react-native-kos-nitro'

const root = path.resolve(__dirname, '..')
const pkg = require(path.join(root, 'package.json'))
const tag = `v${pkg.version}`

const TARGETS = {
  ios: {
    asset: 'KosNitro.xcframework.tar.gz',
    // tarball contains `KosNitro.xcframework/...`
    destDir: path.join(root, 'ios'),
    check: path.join(root, 'ios', 'KosNitro.xcframework'),
  },
  android: {
    asset: 'jniLibs.tar.gz',
    // tarball contains `jniLibs/...`
    destDir: path.join(root, 'android', 'src', 'main'),
    check: path.join(root, 'android', 'src', 'main', 'jniLibs'),
  },
}

function log(msg) {
  console.log(`[kos-nitro] ${msg}`)
}

function isNonEmptyDir(p) {
  try {
    return fs.statSync(p).isDirectory() && fs.readdirSync(p).length > 0
  } catch (_) {
    return false
  }
}

function download(url, dest, redirects = 0) {
  return new Promise((resolve, reject) => {
    if (redirects > 5) return reject(new Error('too many redirects'))
    const req = https.get(
      url,
      { headers: { 'User-Agent': `${REPO}-postinstall` } },
      (res) => {
        if (
          res.statusCode >= 300 &&
          res.statusCode < 400 &&
          res.headers.location
        ) {
          res.resume()
          return resolve(download(res.headers.location, dest, redirects + 1))
        }
        if (res.statusCode !== 200) {
          res.resume()
          return reject(
            new Error(`GET ${url} -> HTTP ${res.statusCode}`)
          )
        }
        const file = fs.createWriteStream(dest)
        res.pipe(file)
        file.on('finish', () => file.close(() => resolve()))
        file.on('error', reject)
      }
    )
    req.on('error', reject)
  })
}

async function fetchTarget(name, t) {
  if (isNonEmptyDir(t.check)) {
    log(`${name}: already present, skipping`)
    return
  }
  const url = `https://github.com/${OWNER}/${REPO}/releases/download/${tag}/${t.asset}`
  const tmp = path.join(os.tmpdir(), `kos-nitro-${name}-${process.pid}.tar.gz`)
  log(`${name}: downloading ${t.asset} (${tag})`)
  await download(url, tmp)
  fs.mkdirSync(t.destDir, { recursive: true })
  log(`${name}: extracting`)
  execFileSync('tar', ['xzf', tmp, '-C', t.destDir])
  fs.unlinkSync(tmp)
  log(`${name}: done`)
}

async function main() {
  if (process.env.KOS_NITRO_SKIP_DOWNLOAD === '1') {
    log('KOS_NITRO_SKIP_DOWNLOAD=1 set, skipping binary download')
    return
  }
  const requested = (process.env.KOS_NITRO_PLATFORMS || 'ios,android')
    .split(',')
    .map((s) => s.trim())
    .filter(Boolean)

  for (const name of requested) {
    const t = TARGETS[name]
    if (!t) {
      log(`unknown platform "${name}", skipping`)
      continue
    }
    await fetchTarget(name, t)
  }
}

main().catch((err) => {
  console.error(
    `[kos-nitro] failed to fetch prebuilt binaries: ${err.message}\n` +
      `  Set KOS_NITRO_SKIP_DOWNLOAD=1 to skip (you must then provide\n` +
      `  ios/KosNitro.xcframework and android/src/main/jniLibs yourself),\n` +
      `  or build them with build_ios.sh / build_android.sh.`
  )
  process.exit(1)
})
