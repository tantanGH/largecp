# LARGECP.X

Large File Fast Copy Tool for Human68k

---

## About This

100MBを超えるような大きいファイルをコピーするのに、COMMAND.X 標準のCOPYコマンドはとても時間がかかります。
バッファリングのためのメモリを大胆に確保することで高速コピーを実現しています。

* 進捗状況の可視化
* 途中キャンセル可能

HUPAIRには対応していません。

---

## インストール

LARCPxxx.ZIP をダウンロードして、LARGECP.X をパスの通ったディレクトリにコピーします。

---

## 使い方

    LARGECP.X - Large file copy utility version 0.1.0 by tantan
    usage: largecp [options] <src-file> <dst-path>
    options:
        -b<n> ... buffer size in MB (1-8, default:4)
        -h    ... show help message

コピー元のファイルと、コピー先のファイル名またはディレクトリ名を指定するだけです。
ワイルドカードには対応していません。

`-b` オプションでバッファメモリサイズを変更できます。
デフォルトは4MBなので、最低でも内蔵メモリ6MB以上が必要です。

---

## 変更履歴

* 0.1.0 (2023/08/13) ... 初版