# Phase 2: YM2151 (Nuked-OPM) 440Hz Tone Generator

このプロジェクトは、Nuked-OPMエミュレータを使用して440Hzの3秒間のWAVファイルを生成するプログラムです。

## 概要

- **内容**: Nuked-OPMを使って、440Hz 3秒のWAVファイルを出力
- **FM音源レジスタwrite後**: 10msのcycle消費を実施
- **プラットフォーム**: Windows + Node.js
- **プログラミング言語**: TypeScript + C (Node.js Native Addon)

## 必要要件

### Windows環境
- Node.js (推奨: v18以上)
- C/C++コンパイラ (以下のいずれか)
  - Visual Studio Build Tools
  - または MinGW-w64

### Linux環境
- Node.js
- gcc または clang
- python3
- make

## セットアップ

1. **依存関係のインストール**
   ```bash
   npm install
   ```

   このコマンドは以下を自動実行します：
   - Node.jsネイティブアドオンのビルド (node-gyp)
   - TypeScriptのコンパイル

2. **手動ビルド（必要に応じて）**
   ```bash
   npm run build
   ```

   このコマンドは以下を実行します：
   - `npm run build:native` - ネイティブアドオンをビルド
   - `npm run build:ts` - TypeScriptをコンパイル

## 使用方法

### 方法1: npmスクリプトを使用

```bash
# デフォルト (output.wavを生成)
npm run generate

# または完全なビルドと実行
npm start
```

### 方法2: Node.jsで直接実行

```bash
# デフォルトのファイル名 (output.wav)
node dist/phase2/index.js

# カスタムファイル名を指定
node dist/phase2/index.js my_tone.wav
```

## 実装の詳細

### アーキテクチャ

```
src/phase2/
├── addon.c         - Node.jsネイティブアドオン (Nuked-OPMラッパー)
├── opm.c           - Nuked-OPMエミュレータコア
├── opm.h           - Nuked-OPMヘッダー
├── index.ts        - TypeScriptメインプログラム
└── README.md       - このファイル

binding.gyp         - node-gypビルド設定
```

### 技術仕様

1. **YM2151の設定**:
   - サンプルレート: 55930 Hz (YM2151の内部サンプリングレート)
   - 周波数: 440Hz (A4音)
   - ブロック: 4
   - FNUM: 290

2. **レジスタ書き込み**:
   - 各レジスタ書き込み後、10msのcycle消費を実装
   - ネイティブアドオン内でOPM_Clockを約35,795回呼び出し (3.579545MHz × 10ms)

3. **WAVファイル形式**:
   - フォーマット: PCM
   - チャンネル: ステレオ (2)
   - ビット深度: 16-bit
   - 長さ: 3秒

### ネイティブアドオンAPI

TypeScriptから以下の関数を呼び出せます：

- `initChip()` - OPMチップを初期化
- `resetChip()` - チップをリセット
- `writeRegister(address, data)` - レジスタに書き込み (10ms分のcycle消費あり)
- `clockChip()` - チップを1サンプル分クロックして音声データを取得
- `cleanupChip()` - チップをクリーンアップ

### Nuked-OPMについて

Nuked-OPMは、Yamaha YM2151 (OPM)チップのサイクル正確なエミュレータです。
- リポジトリ: https://github.com/nukeykt/Nuked-OPM
- ライセンス: LGPL-2.1
- 1980年代から1990年代のアーケードゲームや音源で使用されたFM音源チップ

## トラブルシューティング

### ビルドエラー: "Python not found"

**Windows**: 
- Python 3をインストール: https://www.python.org/downloads/
- 環境変数PATHに追加

**Linux**:
- `sudo apt-get install python3` でインストール

### ビルドエラー: "C compiler not found"

**Windows**: 
- Visual Studio Build Toolsをインストール
- または MinGW-w64をインストール

**Linux**:
- `sudo apt-get install build-essential` でインストール

### 実行エラー: "Cannot find module 'nuked_opm_native.node'"

```bash
npm install
```
を実行して、ネイティブアドオンをビルドしてください。

### 生成されたWAVファイルが再生できない

- ファイルサイズを確認 (約656KB)
- ファイル形式を確認: `file output.wav`
- 異なるメディアプレーヤーで試してください

## ライセンス

- Nuked-OPM: LGPL-2.1 (Copyright (C) 2020 Nuke.YKT)
- このプロジェクト: ISC License
