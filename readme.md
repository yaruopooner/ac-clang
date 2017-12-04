<div id="table-of-contents">
<h2>Table of Contents</h2>
<div id="text-table-of-contents">
<ul>
<li><a href="#sec-1">1. About ac-clang</a></li>
<li><a href="#sec-2">2. Provide Features</a>
<ul>
<li><a href="#sec-2-1">2.1. Basic Features</a></li>
<li><a href="#sec-2-2">2.2. Extended Features</a></li>
<li><a href="#sec-2-3">2.3. Optional Features</a></li>
<li><a href="#sec-2-4">2.4. Other difference</a></li>
</ul>
</li>
<li><a href="#sec-3">3. Installation(external program)</a>
<ul>
<li><a href="#sec-3-1">3.1. Linux &amp; Windows(self-build)</a></li>
<li><a href="#sec-3-2">3.2. Windows(If you use redistributable release binary)</a>
<ul>
<li><a href="#sec-3-2-1">3.2.1. Installation of Visual C++ Redistributable Package</a></li>
<li><a href="#sec-3-2-2">3.2.2. A copy of the external program</a></li>
</ul>
</li>
<li><a href="#sec-3-3">3.3. Precautions</a></li>
</ul>
</li>
<li><a href="#sec-4">4. Installation(lisp package)</a>
<ul>
<li><a href="#sec-4-1">4.1. Required Packages</a></li>
<li><a href="#sec-4-2">4.2. Configuration of ac-clang</a></li>
</ul>
</li>
<li><a href="#sec-5">5. How to use</a>
<ul>
<li><a href="#sec-5-1">5.1. Configuration of libclang flags</a></li>
<li><a href="#sec-5-2">5.2. Configuration of CFLAGS</a></li>
<li><a href="#sec-5-3">5.3. Activation</a></li>
<li><a href="#sec-5-4">5.4. Deactivation</a></li>
<li><a href="#sec-5-5">5.5. Update of libclang flags</a></li>
<li><a href="#sec-5-6">5.6. Update of CFLAGS</a></li>
<li><a href="#sec-5-7">5.7. Debug Logger</a></li>
<li><a href="#sec-5-8">5.8. Profiler</a></li>
<li><a href="#sec-5-9">5.9. Completion</a>
<ul>
<li><a href="#sec-5-9-1">5.9.1. Auto Completion</a></li>
<li><a href="#sec-5-9-2">5.9.2. Manual Completion</a></li>
<li><a href="#sec-5-9-3">5.9.3. BriefComment Display</a></li>
<li><a href="#sec-5-9-4">5.9.4. About types and performance of completion candidate quick help window</a></li>
</ul>
</li>
<li><a href="#sec-5-10">5.10. Jump and return for definition/declaration/inclusion-file</a></li>
</ul>
</li>
<li><a href="#sec-6">6. Limitation</a>
<ul>
<li><a href="#sec-6-1">6.1. Jump for definition(ac-clang-jump-definition / ac-clang-jump-smart) is not perfect.</a></li>
</ul>
</li>
<li><a href="#sec-7">7. Known Issues</a></li>
</ul>
</div>
</div>


[![img](http://melpa.org/packages/ac-clang-badge.svg)](http://melpa.org/#/ac-clang) [![img](http://stable.melpa.org/packages/ac-clang-badge.svg)](http://stable.melpa.org/#/ac-clang)  

[Japanese Manual](./readme.ja.md)  

# About ac-clang<a id="sec-1" name="sec-1"></a>

The original version is emacs-clang-complete-async.  

<https://github.com/Golevka/emacs-clang-complete-async>  

The above fork and it was extended.  

# Provide Features<a id="sec-2" name="sec-2"></a>

The C/C++ Code Completion and the jump to definition/declaration/inclusion-file is provided by libclang.  

![img](./sample-pic-complete.png)  

## Basic Features<a id="sec-2-1" name="sec-2-1"></a>

-   C/C++/Objective-C Code Completion
-   Syntax Check by flymake
-   Jump and return for definition/declaration  
    It isn't necessary to generate a tag file beforehand, and it's possible to jump by on-the-fly.  
    But this feature is not perfect.  
    Because, the case you can't jump well also exists.

## Extended Features<a id="sec-2-2" name="sec-2-2"></a>

The original version is non-implementation.  

-   The number of launch of clang-server is change to 1 process per Emacs.  
    The original is 1 process per buffer.  
    The clang-server is create a session and holds CFLAGS per source code buffer.
-   The template parameters expand  
    Template parameters expand is possible at the time of arguments expand after completion.
-   Completion by manual operation  
    It can be completion at any position.
-   Display brief comment of completion candidate  
    brief comment show on minibuffer.
-   libclang CXTranslationUnit Flags support  
    It is setable on lisp.
-   libclang CXCodeComplete Flags support  
    It is setable on lisp.
-   Multibyte support  
    Modified, because the original is not enough multibyte support.
-   Jump and return for inclusion file.
-   IPC packet format can be specified  
    S-Expression, Json
-   Debug Logger  
    Used for debugging.  
    You are possible to confirm the message and the data that client was sent to clang-server.
-   Performance Profiler  
    Measure the performance of client / server.
-   Miscellaneous  
    Small change and addition

## Optional Features<a id="sec-2-3" name="sec-2-3"></a>

The original version is non-implementation.  
Mainly Windows Platform support.  

-   Project file generation by CMake.  
    Visual Studio Project and Linux Makefile support.
-   Microsoft Visual Studio Platform support  
    clang-server and libclang.dll(clang5.0.0 RELEASE/FINAL) was built by Microsoft Visual Studio 2017/2015/2013
-   x86\_64 Machine Architecture + Windows Platform support  
    Required if you want to completion code for Visual Studio.(for \_WIN64 build support)  
    clang-server and libclang.dll is 64/32bit version.  
    The binary was built by Visual Studio.  
    Therefore, this also has become to those that conform to the machine architecture type of Visual Studio compiler predefined macros.  
    If you build by mingw environment, Visual Studio predefined macros is interfere or not defined.

## Other difference<a id="sec-2-4" name="sec-2-4"></a>

clang-server is described by C++.(The original is C)  

# Installation(external program)<a id="sec-3" name="sec-3"></a>

## Linux & Windows(self-build)<a id="sec-3-1" name="sec-3-1"></a>

Please installation is reference to the manual of clang-server for self-build.  

[Clang Server Manual](./clang-server/readme.md)  

## Windows(If you use redistributable release binary)<a id="sec-3-2" name="sec-3-2"></a>

### Installation of Visual C++ Redistributable Package<a id="sec-3-2-1" name="sec-3-2-1"></a>

If you don't install Visual Studio 2017/2015/2013, required Visual C++ Redistributable Package.  
Please installer gets the vcredist\_x64.exe from following page.  

-   2017  
    <https://www.visualstudio.com/downloads/?q=#other>
-   2015  
    <http://www.microsoft.com/download/details.aspx?id=53587>
-   2013  
    <http://www.microsoft.com/download/details.aspx?id=40784>

### A copy of the external program<a id="sec-3-2-2" name="sec-3-2-2"></a>

<https://github.com/yaruopooner/ac-clang/releases>  

Please download the latest clang-server-X.X.X.zip from above, and unpack to ac-clang directory.  

clang-server.exe  
libclang.dll  
You have to copy this two files to valid path.  
e.g. /usr/local/bin  

## Precautions<a id="sec-3-3" name="sec-3-3"></a>

libclang is not same the LLVM official binary.  
Official libclang has problem that file is locked by LLVM file system used mmap.  
libclang which is being distributed here solved the problem by patch applied to official source code.  
If you want LLVM self-build, you have to apply a patch for solve the above problem.  

# Installation(lisp package)<a id="sec-4" name="sec-4"></a>

## Required Packages<a id="sec-4-1" name="sec-4-1"></a>

Emacs built-in packages and installation required packages.  

-   flymake(built-in)
-   auto-complete
-   pos-tip
-   yasnippet

## Configuration of ac-clang<a id="sec-4-2" name="sec-4-2"></a>

    (require 'ac-clang)
    
    (ac-clang-initialize)

It is complete.  
If you call (ac-clang-initialize), a clang-server will resident.  

If you want to use debug version, the following settings are required before (ac-clang-initialize) execution.  

    (require 'ac-clang)
    
    (ac-clang-server-type 'debug)
    (ac-clang-initialize)

# How to use<a id="sec-5" name="sec-5"></a>

## Configuration of libclang flags<a id="sec-5-1" name="sec-5-1"></a>

It will change the flag of clang-server in the following way  

    (setq ac-clang-clang-translation-unit-flags FLAG-STRING)
    (setq ac-clang-clang-complete-at-flags FLAG-STRING)
    (ac-clang-initialize)

Configuration value is necessary to be set to variable before the initialization function execution.  
Configuration value change after the clang-server launch, uses (ac-clang-update-clang-parameters).  

## Configuration of CFLAGS<a id="sec-5-2" name="sec-5-2"></a>

CFLAGS have to set to variable before ac-clang activation.  

    (setq ac-clang-cflags CFLAGS)

It's set by this.  

## Activation<a id="sec-5-3" name="sec-5-3"></a>

To execute the completion you need to create the source code buffer session on clang-server.  
CFLAGS set to ac-clang-cflags after following execution.  
Run the activate function below after CFLAGS set to ac-clang-cflags.  

    (ac-clang-activate)

Therefore, session associated with buffer is created on clang-server.  

-   Lazy Activation  
    You can delay the activation until the buffer is changed.  
    This is used instead of (ac-clang-activate).  
    
        (ac-clang-activate-after-modify)
    
    If you want to use this activation, it is better to run at c-mode-common-hook.

## Deactivation<a id="sec-5-4" name="sec-5-4"></a>

Delete the session created on clang-server.  

    (ac-clang-deactivate)

## Update of libclang flags<a id="sec-5-5" name="sec-5-5"></a>

It will change the flag of clang-server in the following way  

    (setq ac-clang-clang-translation-unit-flags FLAG-STRING)
    (setq ac-clang-clang-complete-at-flags FLAG-STRING)
    (ac-clang-update-clang-parameters)

Before carrying out this function, the flag of a created session isn't changed.  
A new flag is used for the created session after this function execution.  

## Update of CFLAGS<a id="sec-5-6" name="sec-5-6"></a>

If there is a CFLAGS of updated after the session creation , there is a need to update the CFLAGS of the session .  

    (setq ac-clang-cflags CFLAGS)
    (ac-clang-update-cflags)

When you do this, CFLAGS of the session will be updated.  

This has the same effect.  
But (ac-clang-update-cflags) is small cost than following.  

    (ac-clang-deactivate)
    (ac-clang-activate)

## Debug Logger<a id="sec-5-7" name="sec-5-7"></a>

When you make the following settings  
The contents sent to clang-server are output to a buffer as "**clang-log**".  

    (setq ac-clang-debug-log-buffer-p t)

It will put a limit on the logger buffer size.  
If buffer size larger than designation size, the buffer is cleared.  

    (setq ac-clang-debug-log-buffer-size (* 1024 1000))

If you don't want to be erased a logger buffer, you can set as follows.  

    (setq ac-clang-debug-log-buffer-size nil)

## Profiler<a id="sec-5-8" name="sec-5-8"></a>

When you make the following settings  
Profile result at command execution is output to "**Messages**".  

    (setq ac-clang-debug-profiler-p t)

\#+end\_src  

## Completion<a id="sec-5-9" name="sec-5-9"></a>

### Auto Completion<a id="sec-5-9-1" name="sec-5-9-1"></a>

Completion is executed when the following key input is performed just after the class or the instance object or pointer object.  
-   `.`
-   `->`
-   `::`

If you want to invalidate autocomplete, it will set as follows.  

    (setq ac-clang-async-autocompletion-automatically-p nil)

### Manual Completion<a id="sec-5-9-2" name="sec-5-9-2"></a>

Completion is executed when the following key input is performed.  
-   `<tab>`

Position to perform the key input is the same as auto-completion of the above-mentioned.  
And it is possible completions between word of method or property.  

    struct Foo
    {
        int     m_property0;
        int     m_property1;
    
        void    method( int in )
        {
        }
    };
    
    Foo        foo;
    Foo*       foo0 = &foo;
    
    foo.
    -----
        ^  Execute a manual completion here.
    
    foo->
    ------
         ^  Execute a manual completion here.
    
    Foo::
    ------
         ^  Execute a manual completion here.
    
    foo.m_pro
    ----------
             ^  Execute a manual completion here.

Also, if you want to completion the method of Objective-C/C++, you can only manually completion.  

    id obj = [[NSString alloc] init];
    [obj 
    ------
         ^  Execute a manual completion here.

When manual completion is invalidate or keybind change, it will set as follows.  

    ;; disable
    (setq ac-clang-async-autocompletion-manualtrigger-key nil)
    ;; other key
    (setq ac-clang-async-autocompletion-manualtrigger-key "M-:")

### BriefComment Display<a id="sec-5-9-3" name="sec-5-9-3"></a>

It is displayed by default setting.  
To invalidate the display, remove the BriefComment flag from the following variables.  

The flags of BriefComment are as follows.  
`ac-clang-clang-translation-unit-flags` is `CXTranslationUnit_IncludeBriefCommentsInCodeCompletion`  
`ac-clang-clang-complete-at-flags` is `CXCodeComplete_IncludeBriefComments`  

### About types and performance of completion candidate quick help window<a id="sec-5-9-4" name="sec-5-9-4"></a>

The quick help window displays argument information etc of completion candidate.  
There are two quick help window, popup.el and pos-tip.el.  
By default, popup is used.  
To change the popup window, set as follows.  

    ;; popup(default)
    (setq ac-clang-quick-help-prefer-pos-tip-p nil)
    ;; pos-tip
    (setq ac-clang-quick-help-prefer-pos-tip-p t)

-   popup  
    Although it is lightweight and scroll response is also good, the window may occasionally shift.
-   pos-tip  
    I am using x-show-tip, and it looks nice and looks rich.  
    But scroll behavior is heavyweight.  
    Scrolling performance will degrade if you scroll by a large amount with a lot of completion candidates.

## Jump and return for definition/declaration/inclusion-file<a id="sec-5-10" name="sec-5-10"></a>

In the activated buffer, you move the cursor at word that want to jump.  
Execute following, you can jump to the source file that the class / method / function / enum / macro did definition or declaration.  
This is possible jump to inclusion file.  

    (ac-clang-jump-smart)

Keybind is "M-,"  

The return operation is possible in the following.  

    (ac-clang-jump-back)

Keybind is "M-,"  

The jump history is stacked, enabling continuous jump and continuous return.  
If you execute jump operation in non-activation buffer, that buffer is automatically activated and jump.  

-   `(ac-clang-jump-smart)`  
    1st priority jump location is the definition.  
    But if the definition is not found, it will jump to the declaration.  
    Jump to inclusion-file.( Please run the command on the `#include` keyword )
-   `(ac-clang-jump-inclusion)`  
         Jump to inclusion-file.
-   `(ac-clang-jump-definition)`  
         Jump to definition.
-   `(ac-clang-jump-declaration)`  
         Jump to declaration.

# Limitation<a id="sec-6" name="sec-6"></a>

## Jump for definition(ac-clang-jump-definition / ac-clang-jump-smart) is not perfect.<a id="sec-6-1" name="sec-6-1"></a>

The function / class method are subject to restrictions.  
struct/class/typedef/template/enum/class-variable/global-variable/macro/preprocessor don't have problem.  
libclang analyze current buffer and including headers by current buffer, and decide jump location from result.  
Therefore, when function definition and class method definition is described in including headers, it is possible to jump.  
If it is described in c / cpp, it is impossible to jump. Because libclang can't know c / cpp.  
1st priority jump location is the declaration.  
But if the declaration is not found, it will jump to the definition.  
When emphasizing a definition jump, I'll recommend you use with GNU global(GTAGS).  

# Known Issues<a id="sec-7" name="sec-7"></a>

nothing
