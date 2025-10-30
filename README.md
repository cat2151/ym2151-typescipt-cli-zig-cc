# YM2151 TypeScript CLI with zig cc

YM2151 (Nuked-OPM) FM音源エミュレータを使用した音声生成プロジェクト

## プロジェクト構成

- **src/phase2/** - Nuked-OPMを使用した440Hz 3秒WAVファイル生成プログラム

## クイックスタート

```bash
# 依存関係のインストール
npm install

# ビルド (zig ccまたはgccが必要)
npm run build

# 440Hz音声ファイルの生成
npm run generate
```

詳細は [src/phase2/README.md](src/phase2/README.md) を参照してください。

## 必要要件

- Node.js (v18以上推奨)
- zig (Windows) または gcc (Linux)

## 技術スタック

- **TypeScript** - メインプログラミング言語
- **zig cc** - Cコンパイラ (mingw不使用)
- **Nuked-OPM** - YM2151エミュレータライブラリ

## ライセンス

ISC License (Nuked-OPMはLGPL-2.1)