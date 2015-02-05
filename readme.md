<div id="table-of-contents">
<h2>Table of Contents</h2>
<div id="text-table-of-contents">
<ul>
<li><a href="#sec-1">1. ac-clang について</a></li>
<li><a href="#sec-2">2. 提供される機能</a>
<ul>
<li><a href="#sec-2-1">2.1. 基本機能</a></li>
<li><a href="#sec-2-2">2.2. 拡張機能</a></li>
<li><a href="#sec-2-3">2.3. オプション機能</a></li>
<li><a href="#sec-2-4">2.4. その他差異</a></li>
</ul>
</li>
<li><a href="#sec-3">3. インストール(external program)</a>
<ul>
<li><a href="#sec-3-1">3.1. Linux &amp; Windows(self-build)</a></li>
<li><a href="#sec-3-2">3.2. Windows(配布用releaseバイナリ利用の場合)</a>
<ul>
<li><a href="#sec-3-2-1">3.2.1. Visual C++ 再頒布可能パッケージのインストール</a></li>
<li><a href="#sec-3-2-2">3.2.2. 外部プログラムのコピー</a></li>
</ul>
</li>
<li><a href="#sec-3-3">3.3. 注意事項</a></li>
</ul>
</li>
<li><a href="#sec-4">4. インストール(lisp package)</a>
<ul>
<li><a href="#sec-4-1">4.1. ac-clang の設定</a></li>
</ul>
</li>
<li><a href="#sec-5">5. 使用方法</a>
<ul>
<li><a href="#sec-5-1">5.1. libclang各種フラグ設定</a></li>
<li><a href="#sec-5-2">5.2. CFLAGSの設定</a></li>
<li><a href="#sec-5-3">5.3. アクティブ化</a></li>
<li><a href="#sec-5-4">5.4. 非アクティブ化</a></li>
<li><a href="#sec-5-5">5.5. libclang各種フラグ更新</a></li>
<li><a href="#sec-5-6">5.6. CFLAGSの更新</a></li>
<li><a href="#sec-5-7">5.7. デバッグロガー</a></li>
<li><a href="#sec-5-8">5.8. 定義/宣言へのジャンプ＆リターン</a></li>
</ul>
</li>
<li><a href="#sec-6">6. 制限事項</a>
<ul>
<li><a href="#sec-6-1">6.1. 補完対象に対してアクセス指定子が考慮されない</a></li>
<li><a href="#sec-6-2">6.2. 定義ジャンプ(ac-clang:jump-definition / ac-clang:jump-smart)が完全ではない</a></li>
</ul>
</li>
<li><a href="#sec-7">7. 既知の不具合</a></li>
</ul>
</div>
</div>



# ac-clang について<a id="sec-1" name="sec-1"></a>

オリジナル版はemacs-clang-complete-async  

<https://github.com/Golevka/emacs-clang-complete-async>  

上記をforkして独自拡張したもの。  

# 提供される機能<a id="sec-2" name="sec-2"></a>

libclang を利用してC/C++コード補完と宣言/定義へのジャンプを行います。  
基本機能はemacs-clang-complete-asyncと同じです。  
※実装方法は変更されているものがあります。  

![img](./sample-pic-complete.png)  

## 基本機能<a id="sec-2-1" name="sec-2-1"></a>

-   C/C++コード補完
-   flymakeによるシンタックスチェック
-   宣言/定義へのジャンプ＆リターン  
    GTAGSのlibclang版  
    事前にタグファイルを生成する必要がなくオンザフライでジャンプ可能

## 拡張機能<a id="sec-2-2" name="sec-2-2"></a>

オリジナル版は非実装  

-   clang-serverをEmacs１つにつき１プロセスに変更  
    オリジナルは１バッファにつき１プロセス。  
    clang-serverはプロセス内でソースコードバッファ毎にセッションを作成してCFLAGS等を保持します。
-   テンプレートパラメーター展開をサポート  
    補完後の引数展開時にテンプレートパラメーター展開が可能
-   libclang CXTranslationUnit Flagsをサポート  
    lispから設定可能
-   libclang CXCodeComplete Flagsをサポート  
    lispから設定可能
-   マルチバイトサポート  
    オリジナルはマルチバイトサポートが完全ではなかったので修正
-   デバッグロガーサポート  
    デバッグ用途で使用  
    clang-serverに送信するメッセージとデータをプールして確認可能
-   その他  
    微細な追加or変更

## オプション機能<a id="sec-2-3" name="sec-2-3"></a>

オリジナル版は非実装  
主にWindows Platform サポート  

-   CMake によるプロジェクト生成  
    Visual Studio用プロジェクトと Linux用Makefileを生成可能
-   Microsoft Visual Studio プラットフォームサポート  
    clang-server と libclang.dll(clang3.5.0 RELEASE/FINAL) を  
    Microsoft Visual Studio 2013 でビルド
-   x86\_64 Machine Architecture + Windows Platform サポート  
    Visual Studio用コードを補完する場合は必須。(\_WIN64 ビルドサポートのため)  
    clang-serverとlibclang.dllは64bit版。  
    x86\_32はclang3.5から未サポートにしました。  
    Visual Studioでビルドされているのでコンパイラ定義済みマクロも  
    Visual Studioのマシンアーキテクチャタイプに準拠したものになっています。  
    ※mingwによるビルドだとVisual Studio定義済みマクロ等が定義されなかったり干渉したりする。

## その他差異<a id="sec-2-4" name="sec-2-4"></a>

clang-serverはC++で記述（オリジナルはC）  

# インストール(external program)<a id="sec-3" name="sec-3"></a>

## Linux & Windows(self-build)<a id="sec-3-1" name="sec-3-1"></a>

セルフビルドによるインストールはclang-serverのマニュアルを参考にしてください。  

[Clang Server Manual](./clang-server/readme.md)  

## Windows(配布用releaseバイナリ利用の場合)<a id="sec-3-2" name="sec-3-2"></a>

### Visual C++ 再頒布可能パッケージのインストール<a id="sec-3-2-1" name="sec-3-2-1"></a>

Visual Studio 2013がインストールされていない環境では  
Visual C++ 再頒布可能パッケージが必要になります。  
以下のページからvcredist\_x64.exeを取得しインストールしてください。  

<http://www.microsoft.com/download/details.aspx?id=40784>  

### 外部プログラムのコピー<a id="sec-3-2-2" name="sec-3-2-2"></a>

<https://github.com/yaruopooner/ac-clang/releases>  

上記からclang-server-X.X.X.zipをダウンロードしてac-clangに解凍してください。  

ac-clang/clang-server/binary/clang-server-x86\_64.exe  
ac-clang/clang-server/library/x86\_64/release/libclang.dll  
上記２ファイルをパスの通っている場所へコピーします。  
※たとえば /usr/local/bin など  

-   64bit version  
    clang-server-x86\_64.exe  
    libclang.dll
-   <del>32bit version</del>  
    clang3.5から未サポートにしました。  
    <del>clang-server-x86\_32.exe</del>  
    <del>libclang.dll</del>

## 注意事項<a id="sec-3-3" name="sec-3-3"></a>

libclangはLLVMオフィシャルのバイナリと異なります。  
オフィシャルのlibclangはLLVMファイルシステム内で使用されるmmapがファイルをロックしてしまう問題があります。  
ここで配布しているlibclangはオフィシャルソースコードにパッチを当てて問題を解決したバイナリです。  
またLLVMセルフビルド時も上記の問題を解決するパッチを適用します。  

# インストール(lisp package)<a id="sec-4" name="sec-4"></a>

## ac-clang の設定<a id="sec-4-1" name="sec-4-1"></a>

    (require 'ac-clang)
    
    (ac-clang:initialize)

以上で完了です。  
(ac-clang:initialize) を呼び出すと clang-server-x86\_64 が常駐します。  

32bit 版を使用する場合は (ac-clang:initialize) 実行前に以下の設定が必要です。  

    (require 'ac-clang)
    
    (ac-clang:server-type 'x86_32)
    (ac-clang:initialize)

# 使用方法<a id="sec-5" name="sec-5"></a>

## libclang各種フラグ設定<a id="sec-5-1" name="sec-5-1"></a>

以下の方法で clang-server のフラグを変更します  

    (setq ac-clang:clang-translation-unit-flags FLAG-STRING)
    (setq ac-clang:clang-complete-at-flags FLAG-STRING)
    (ac-clang:initialize)

初期化関数実行より前に変数にセットされている必要があります。  
clang-server起動後の変更は後述の (ac-clang:update-clang-parameters) を利用します。  

## CFLAGSの設定<a id="sec-5-2" name="sec-5-2"></a>

ac-clangをアクティブ化する前にCFLAGSをセットしておく必要があります。  

    (setq ac-clang:cflags CFLAGS)

でセットします。  

## アクティブ化<a id="sec-5-3" name="sec-5-3"></a>

補完を行うには clang-server で該当バッファのセッションを作成する必要があります。  
ac-clang:cflags に CFLAGS がセットされた状態で  

    (ac-clang:activate)

を実行します。  
これにより clang-server にバッファに関連付けされたセッションが作成されます。  

-   アクティブ化の遅延  
    バッファが変更されるまでアクティブ化を遅延させることができます。  
    
        (ac-clang:activate)
    
    の変わりに  
    
        (ac-clang:activate-after-modify)
    
    を使います。  
    c-mode-common-hook などで実行する場合はこれを使うとよいでしょう。

## 非アクティブ化<a id="sec-5-4" name="sec-5-4"></a>

clang-server で作成されたセッションを破棄します。  

    (ac-clang:deactivate)

## libclang各種フラグ更新<a id="sec-5-5" name="sec-5-5"></a>

以下の方法で clang-server のフラグを変更します  

    (setq ac-clang:clang-translation-unit-flags FLAG-STRING)
    (setq ac-clang:clang-complete-at-flags FLAG-STRING)
    (ac-clang:update-clang-parameters)

この関数を実行する前に作成されたセッションのフラグは変更されません。  
関数実行後に作成されるセッションのフラグは新しくセットしたものが利用されます。  

## CFLAGSの更新<a id="sec-5-6" name="sec-5-6"></a>

セッション作成後にCFLAGSの更新があった場合はセッションのCFLAGSを更新する必要があります。  

    (setq ac-clang:cflags CFLAGS)
    (ac-clang:update-cflags)

と実行することにより、セッションのCFLAGSが更新されます。  

※以下の方法でも同じ効果になりますが、 (ac-clang:update-cflags) を実行するほうがコストは安いです。  

    (ac-clang:deactivate)
    (ac-clang:activate)

## デバッグロガー<a id="sec-5-7" name="sec-5-7"></a>

以下の設定を行うと  
clang-serverに送信した内容が "**clang-log**" というバッファに出力されます。  

    (setq ac-clang:debug-log-buffer-p t)

ロガーバッファサイズに制限をかけます。  
バッファが指定サイズ以上になるとクリアされます。  

    (setq ac-clang:debug-log-buffer-size (* 1024 1000))

クリアせず無制限にする場合は以下のように設定します。  

    (setq ac-clang:debug-log-buffer-size nil)

## 定義/宣言へのジャンプ＆リターン<a id="sec-5-8" name="sec-5-8"></a>

アクティブ化されたバッファ上でジャンプしたいワード上にカーソルをポイントして以下を実行すると、  
クラス/メソッド/関数/enumなどが定義/宣言されているソースファイルへジャンプすることが出来ます。  

    (ac-clang::jump-smart)

"M-." にバインドされています。  

リターン操作は以下で可能です。  

    (ac-clang:jump-back)

"M-," にバインドされています。  

ジャンプ履歴はスタックされており、連続ジャンプ・連続リターンが可能です。  

※アクティブ化されていないバッファ上でジャンプ操作を実行した場合  
  該当バッファは自動的にアクティブ化されジャンプを行います。  

-   `(ac-clang::jump-smart)`  
         定義優先でジャンプしますが定義が見つからない場合は宣言へジャンプします。
-   `(ac-clang::jump-declaration)`  
         宣言へジャンプします。
-   `(ac-clang::jump-definition)`  
         定義へジャンプします。

# 制限事項<a id="sec-6" name="sec-6"></a>

## 補完対象に対してアクセス指定子が考慮されない<a id="sec-6-1" name="sec-6-1"></a>

クラス変数・クラスメソッドは全てpublicアクセス指定子扱いで補完対象としてリストアップされる。  

## 定義ジャンプ(ac-clang:jump-definition / ac-clang:jump-smart)が完全ではない<a id="sec-6-2" name="sec-6-2"></a>

関数とクラスメソッドに関してのみ制限があります。  
struct/class/typedef/template/enum/class variable/global variableなどは問題ありません。  
libclang は現在編集中のバッファと、それらからincludeされるヘッダファイルからジャンプ先を決定している。  
このため、関数定義やクラスメソッド定義がincludeされるヘッダファイルに記述されている場合はジャンプ可能だが、  
c/cppファイルに記述されている場合はlibclangがc/cppファイルを収集する術が無いのでジャンプできない。  
※ ac-clang:jump-smart は定義優先でジャンプしますが定義が見つからない場合は宣言へジャンプします。  
定義ジャンプを重視する場合はGTAGSなどと併用をお勧めします。  

# 既知の不具合<a id="sec-7" name="sec-7"></a>

なし
