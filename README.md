# YM2151 TypeScript CLI with Native Addon

YM2151 (Nuked-OPM) FM音源エミュレータを使用した音声生成プロジェクト

## プロジェクト構成

- **src/phase2/** - Nuked-OPMを使用した440Hz 3秒WAVファイル生成プログラム
- **binding.gyp** - Node.jsネイティブアドオンのビルド設定

## クイックスタート

```bash
# 依存関係のインストール (自動的にネイティブアドオンをビルド)
npm install

# 440Hz音声ファイルの生成
npm run generate
```

詳細は [src/phase2/README.md](src/phase2/README.md) を参照してください。

## 必要要件

- Node.js (v18以上推奨)
- C/C++コンパイラ (gcc, clang, MSVC等)
- Python 3 (node-gypに必要)

## 技術スタック

- **TypeScript** - メインプログラミング言語
- **Node.js Native Addon (N-API)** - Cコードとの連携
- **Nuked-OPM** - YM2151エミュレータライブラリ (LGPL-2.1)
- **node-gyp** - ネイティブアドオンビルドツール

## 実装の特徴

- TypeScriptから直接OPMレジスタ書き込み関数を呼び出し
- レジスタ書き込み後に10ms分のOPM cycleを消費 (約35,795サイクル)
- 実行ファイル (exe) は生成せず、ネイティブライブラリとして実装

## ライセンス

ISC License (Nuked-OPMはLGPL-2.1)