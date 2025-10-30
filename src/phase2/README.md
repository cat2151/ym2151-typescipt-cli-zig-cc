# Phase 2: YM2151 (Nuked-OPM) 440Hz Tone Generator

このプロジェクトは、Nuked-OPMエミュレータを使用して440Hzの3秒間のWAVファイルを生成するプログラムです。

## 概要

- **内容**: Nuked-OPMを使って、440Hz 3秒のWAVファイルを出力
- **FM音源レジスタwrite後**: 10msのcycle消費を実施
- **プラットフォーム**: Windows + Node.js
- **プログラミング言語**: TypeScript + zig cc (Cコンパイラ)

## 必要要件

### Windows環境
- Node.js (推奨: v18以上)
- zig (0.11.0以上)
  - ダウンロード: https://ziglang.org/download/

### Linuxでテストする場合
- Node.js
- zig または gcc

## セットアップ

1. **zig のインストール (Windows)**
   ```powershell
   # zigをダウンロードして解凍
   # 環境変数PATHに追加してください
   ```

2. **依存関係のインストール**
   ```bash
   npm install
   ```

3. **ビルド**
   ```bash
   npm run build
   ```

   このコマンドは以下を実行します：
   - `npm run build:native` - zig ccを使用してC言語プログラムをコンパイル
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

### 方法3: C実行ファイルを直接実行

```bash
# Windows
src\phase2\opm_generator.exe output.wav

# Linux
./src/phase2/opm_generator output.wav
```

## 実装の詳細

### アーキテクチャ

```
src/phase2/
├── main.c          - Nuked-OPMを使用するCプログラム
├── opm.c           - Nuked-OPMエミュレータコア
├── opm.h           - Nuked-OPMヘッダー
├── index.ts        - TypeScriptインターフェース
├── build.sh        - Linuxビルドスクリプト
├── build.bat       - Windowsビルドスクリプト
└── README.md       - このファイル
```

### 技術仕様

1. **YM2151の設定**:
   - サンプルレート: 55930 Hz (YM2151の内部サンプリングレート)
   - 周波数: 440Hz (A4音)
   - ブロック: 4
   - FNUM: 290

2. **レジスタ書き込み**:
   - 各レジスタ書き込み後、10msの遅延を実装
   - `sleep_ms(10)` 関数で実装

3. **WAVファイル形式**:
   - フォーマット: PCM
   - チャンネル: ステレオ (2)
   - ビット深度: 16-bit
   - 長さ: 3秒

### Nuked-OPMについて

Nuked-OPMは、Yamaha YM2151 (OPM)チップのサイクル正確なエミュレータです。
- リポジトリ: https://github.com/nukeykt/Nuked-OPM
- ライセンス: LGPL-2.1
- 1980年代から1990年代のアーケードゲームや音源で使用されたFM音源チップ

## トラブルシューティング

### ビルドエラー: "zig not found"

**Windows**: 
- zigをダウンロードしてインストール: https://ziglang.org/download/
- 環境変数PATHに追加

**Linux**:
- 代替としてgccが自動的に使用されます

### 実行エラー: "Executable not found"

```bash
npm run build
```
を実行して、C実行ファイルをビルドしてください。

### 生成されたWAVファイルが再生できない

- ファイルサイズを確認 (約656KB)
- ファイル形式を確認: `file output.wav`
- 異なるメディアプレーヤーで試してください

## ライセンス

- Nuked-OPM: LGPL-2.1 (Copyright (C) 2020 Nuke.YKT)
- このプロジェクト: ISC License
