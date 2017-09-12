;;; ac-clang.el --- Auto Completion source by libclang for GNU Emacs -*- lexical-binding: t; -*-

;;; last updated : 2017/09/12.18:39:37

;; Copyright (C) 2010       Brian Jiang
;; Copyright (C) 2012       Taylan Ulrich Bayirli/Kammer
;; Copyright (C) 2013       Golevka
;; Copyright (C) 2013-2017  yaruopooner
;; 
;; Original Authors: Brian Jiang <brianjcj@gmail.com>
;;                   Golevka [https://github.com/Golevka]
;;                   Taylan Ulrich Bayirli/Kammer <taylanbayirli@gmail.com>
;;                   Many others
;; Author: yaruopooner [https://github.com/yaruopooner]
;; URL: https://github.com/yaruopooner/ac-clang
;; Keywords: completion, convenience, intellisense
;; Version: 1.9.2
;; Package-Requires: ((emacs "24") (cl-lib "0.5") (auto-complete "1.4.0") (pos-tip "0.4.6") (yasnippet "0.8.0"))


;; This program is free software; you can redistribute it and/or modify
;; it under the terms of the GNU General Public License as published by
;; the Free Software Foundation, either version 3 of the License, or
;; (at your option) any later version.
;; 
;; This program is distributed in the hope that it will be useful,
;; but WITHOUT ANY WARRANTY; without even the implied warranty of
;; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;; GNU General Public License for more details.
;; 
;; You should have received a copy of the GNU General Public License
;; along with this program.  If not, see <http://www.gnu.org/licenses/>.


;;; Commentary:
;; 
;; * INTRODUCTION:
;;   This program fork from auto-complete-clang-async.el
;;   ac-clang provide code completion and arguments expand.
;;   This program consists of the client(elisp) and server(binary).
;;   The server is executable file, and a self-build is necessary.
;;   The server achieve code completion using libclang of LLVM.
;; 
;; * FEATURES:
;;   - Basic(same auto-complete-clang-async)
;;     Code Completion by libclang.
;;     Auto Completion support. 
;;     Uses a "completion server" process to utilize libclang.
;;     C/C++/Objective-C mode support.
;;     Jump to definition or declaration. return from jumped location. 
;;     Jump is an on-the-fly that doesn't use the tag file.
;;     Also provides flymake syntax checking.
;;     A few bugfix and refactoring.
;;    
;;   - Extension
;;     "completion server" process is 1 process per Emacs. (original version is per buffer)
;;     Template Method Parameters expand support. 
;;     Manual Completion support. 
;;     libclang CXTranslationUnit Flags support. 
;;     libclang CXCodeComplete Flags support. 
;;     Multibyte support. 
;;     Debug Logger Buffer support. 
;;     Jump to inclusion-file. return from jumped location. 
;;     more a few modified. (client & server)
;;    
;;   - Optional
;;     CMake support.
;;     clang-server.exe and libclang.dll built with Microsoft Visual Studio 2017/2015/2013.
;;     x86_64 Machine Architecture + Windows Platform support. (Visual Studio Predefined Macros)
;; 
;; * EASY INSTALLATION(Windows Only):
;;   - Visual C++ Redistributable Packages for Visual Studio 2017/2015/2013
;;     Must be installed if don't have a Visual Studio 2017/2015/2013.
;; 
;;     - 2017
;;       [https://download.microsoft.com/download/e/4/f/e4f8372f-ef78-4afa-a418-c6633a49770c/vc_redist.x64.exe]
;;       [https://download.microsoft.com/download/d/f/d/dfde0309-51a2-4722-a848-95fb06ec57d1/vc_redist.x86.exe]
;;     - 2015
;;       [https://download.microsoft.com/download/9/3/F/93FCF1E7-E6A4-478B-96E7-D4B285925B00/vc_redist.x64.exe]
;;       [https://download.microsoft.com/download/9/3/F/93FCF1E7-E6A4-478B-96E7-D4B285925B00/vc_redist.x86.exe]
;;     - 2013/2012/2010/2008
;;       [http://www.standaloneofflineinstallers.com/2015/12/Microsoft-Visual-C-Redistributable-2015-2013-2012-2010-2008-2005-32-bit-x86-64-bit-x64-Standalone-Offline-Installer-for-Windows.html]
;;    
;;   - Completion Server Program
;;     Built with Microsoft Visual Studio 2017/2015/2013.
;;     [https://github.com/yaruopooner/ac-clang/releases]
;;     1. download clang-server.zip
;;     2. clang-server.exe and libclang.dll is expected to be available in the PATH or in Emacs' exec-path.
;;    
;; * STANDARD INSTALLATION(Linux, Windows):
;;   Generate a Unix Makefile or a Visual Studio Project by CMake.
;; 
;;   - Self-Build step
;;     1. LLVM
;;        checkout, apply patch, generate project, build
;;        It is recommended that you use this shell.
;;        [https://github.com/yaruopooner/llvm-build-shells.git]
;; 
;;     2. Clang Server
;;        generate project, build
;; 
;;     see clang-server's reference manual.
;;     ac-clang/clang-server/readme.org
;; 
;; * NOTICE:
;;   - LLVM libclang.[dll, so, ...]
;;     This binary is not official binary.
;;     Because offical libclang has mmap lock problem.
;;     Applied a patch to LLVM's source code in order to solve this problem.
;; 
;;     see clang-server's reference manual.
;;     ac-clang/clang-server/readme.org
;; 


;; Usage:
;; * DETAILED MANUAL:
;;   For more information and detailed usage, refer to the project page:
;;   [https://github.com/yaruopooner/ac-clang]
;; 
;; * SETUP:
;;   (require 'ac-clang)
;; 
;;   (setq w32-pipe-read-delay 0)          ;; <- Windows Only
;; 
;;   (when (ac-clang-initialize)
;;     (add-hook 'c-mode-common-hook '(lambda ()
;;                                      (setq ac-clang-cflags CFLAGS)
;;                                      (ac-clang-activate-after-modify))))
;; 
;; * DEFAULT KEYBIND
;;   - start auto completion
;;     code completion & arguments expand
;;     `.` `->` `::`
;;   - start manual completion
;;     code completion & arguments expand
;;     `<tab>`
;;   - jump to inclusion-file, definition, declaration / return from it
;;     this is nestable jump.
;;     `M-.` / `M-,`
;; 

;;; Code:



(require 'cl-lib)
(require 'auto-complete)
(require 'json)
(require 'pos-tip)
(require 'yasnippet)
(require 'flymake)




(defconst ac-clang-version "1.9.2")
(defconst ac-clang-libclang-version nil)


;;;
;;; for Server vars
;;;


;; clang-server binary type
(defvar ac-clang-server-type 'release
  "clang-server binary type
`release'  : release build version
`debug'    : debug build version (server develop only)
`x86_64'   : (obsolete. It will be removed in the future.) 64bit release build version
`x86_64d'  : (obsolete. It will be removed in the future.) 64bit debug build version (server develop only)
`x86_32'   : (obsolete. It will be removed in the future.) 32bit release build version
`x86_32d'  : (obsolete. It will be removed in the future.) 32bit debug build version (server develop only)
")


;; clang-server launch option values
(defvar ac-clang-server-stdin-buffer-size nil
  "STDIN buffer size. value range is 1 - 5 MB. 
If the value is nil, will be allocated 1MB.
The value is specified in MB.")

(defvar ac-clang-server-stdout-buffer-size nil
  "STDOUT buffer size. value range is 1 - 5 MB. 
If the value is nil, will be allocated 1MB.
The value is specified in MB.")

(defvar ac-clang-server-logfile nil
  "IPC records output file.(for debug)")


;; server binaries property list
(defconst ac-clang--server-binaries '(release "clang-server"
                                      debug   "clang-server-debug"))

(defconst ac-clang--server-obsolete-binaries '(x86_64  "clang-server-x86_64"
                                               x86_64d "clang-server-x86_64d"
                                               x86_32  "clang-server-x86_32"
                                               x86_32d "clang-server-x86_32d"))


;; server process details
(defcustom ac-clang--server-executable nil
  "Location of clang-server executable."
  :group 'auto-complete
  :type 'file)


(defconst ac-clang--process-name "Clang-Server")

(defconst ac-clang--process-buffer-name "*Clang-Server*")
(defconst ac-clang--completion-buffer-name "*Clang-Completion*")
(defconst ac-clang--diagnostics-buffer-name "*Clang-Diagnostics*")

(defvar ac-clang--server-process nil)
(defvar ac-clang--status 'idle
  "clang-server status
`idle'          : job is nothing
`receive'       : receiving command sent result
`transaction'   : transaction execute to received command result
`shutdown'      : shutdown server
  ")


(defvar ac-clang--activate-buffers nil)


;; server debug
(defconst ac-clang--debug-log-buffer-name "*Clang-Log*")
(defvar ac-clang-debug-log-buffer-p nil)
(defvar ac-clang-debug-log-buffer-size (* 1024 50))


;; clang-server behaviors
(defvar ac-clang-clang-translation-unit-flags "CXTranslationUnit_DetailedPreprocessingRecord|CXTranslationUnit_PrecompiledPreamble|CXTranslationUnit_CacheCompletionResults|CXTranslationUnit_CreatePreambleOnFirstParse"
  "CXTranslationUnit Flags. 
for Server behavior.
The value sets flag-name strings or flag-name combined strings.
Separator is `|'.
`CXTranslationUnit_DetailedPreprocessingRecord'            : Required if you want jump to macro declaration, inclusion-file.
`CXTranslationUnit_Incomplete'                             :  
`CXTranslationUnit_PrecompiledPreamble'                    : Increase completion performance.
`CXTranslationUnit_CacheCompletionResults'                 : Increase completion performance.
`CXTranslationUnit_ForSerialization'                       :  
`CXTranslationUnit_CXXChainedPCH'                          :  
`CXTranslationUnit_SkipFunctionBodies'                     :  
`CXTranslationUnit_IncludeBriefCommentsInCodeCompletion'   : Required if you want to brief-comment of completion.
`CXTranslationUnit_CreatePreambleOnFirstParse'             : Increase completion performance.
`CXTranslationUnit_KeepGoing'                              : 
")

(defvar ac-clang-clang-complete-at-flags "CXCodeComplete_IncludeMacros"
  "CXCodeComplete Flags. 
for Server behavior.
The value sets flag-name strings or flag-name combined strings.
Separator is `|'.
`CXCodeComplete_IncludeMacros'
`CXCodeComplete_IncludeCodePatterns'
`CXCodeComplete_IncludeBriefComments'
")

(defvar ac-clang-clang-complete-results-limit 0
  "acceptable number of result candidate. 
for Server behavior.
ac-clang-clang-complete-results-limit == 0 : accept all candidates.
ac-clang-clang-complete-results-limit != 0 : if number of result candidates greater than ac-clang-clang-complete-results-limit, discard all candidates.
")


;; client behaviors
(defvar ac-clang-tmp-pch-automatic-cleanup-p (eq system-type 'windows-nt)
  "automatically cleanup for generated temporary precompiled headers.")


(defvar ac-clang-server-automatic-recovery-p t
  "automatically recover server when command queue reached limitation.")



;;;
;;; for auto-complete vars
;;;

;; clang-server response filter pattern for auto-complete candidates
(defconst ac-clang--completion-pattern "^COMPLETION: \\(%s[^\s\n:]*\\)\\(?: : \\)*\\(.*$\\)")

;; auto-complete behaviors
(defvar ac-clang-async-autocompletion-automatically-p t
  "If autocompletion is automatically triggered when you type `.', `->', `::'")

(defvar ac-clang-async-autocompletion-manualtrigger-key "<tab>")



;; auto-complete faces
(defface ac-clang-candidate-face
  '((t (:background "lightgray" :foreground "navy")))
  "Face for clang candidate"
  :group 'auto-complete)

(defface ac-clang-selection-face
  '((t (:background "navy" :foreground "white")))
  "Face for the clang selected candidate."
  :group 'auto-complete)



;;;
;;; for Session vars
;;;

(defvar-local ac-clang--activate-p nil)

(defvar-local ac-clang--session-name nil)

;; for patch
(defvar-local ac-clang--suspend-p nil)


;; auto-complete ac-sources backup
(defvar-local ac-clang--ac-sources-backup nil)


;; auto-complete candidates and completion start-point
(defvar-local ac-clang--candidates nil)
(defvar-local ac-clang--start-point nil)
(defvar-local ac-clang--template-candidates nil)
(defvar-local ac-clang--template-start-point nil)


;; CFLAGS build behaviors
(defvar-local ac-clang-language-option-function nil
  "Function to return the language type for option -x.")

(defvar-local ac-clang-prefix-header nil
  "The prefix header to pass to the Clang executable.")


;; clang-server session behavior
(defvar-local ac-clang-cflags nil
  "Extra flags to pass to the Clang executable.
This variable will typically contain include paths, e.g., (\"-I~/MyProject\" \"-I.\").")


(defvar ac-clang--jump-stack nil
  "The jump stack (keeps track of jumps via jump-inclusion, jump-definition, jump-declaration, jump-smart)") 




;;;
;;; primitive functions
;;;

;; server launch option builder
(defun ac-clang--build-server-launch-options ()
  (append 
   (when ac-clang-server-stdin-buffer-size
     (list "--stdin-buffer-size" (format "%d" ac-clang-server-stdin-buffer-size)))
   (when ac-clang-server-stdout-buffer-size
     (list "--stdout-buffer-size" (format "%d" ac-clang-server-stdout-buffer-size)))
   (when ac-clang-server-logfile
     (list "--logfile" (format "%s" ac-clang-server-logfile)))))


;; CFLAGS builders
(defsubst ac-clang--language-option ()
  (or (and ac-clang-language-option-function
           (funcall ac-clang-language-option-function))
      (cond ((eq major-mode 'c++-mode)
             "c++")
            ((eq major-mode 'c-mode)
             "c")
            ((eq major-mode 'objc-mode)
             (cond ((string= "m" (file-name-extension (buffer-file-name)))
                    "objective-c")
                   (t
                    "objective-c++")))
            (t
             "c++"))))


(defsubst ac-clang--build-complete-cflags ()
  (append '("-cc1" "-fsyntax-only")
          (list "-x" (ac-clang--language-option))
          ac-clang-cflags
          (when (stringp ac-clang-prefix-header)
            (list "-include-pch" (expand-file-name ac-clang-prefix-header)))))



;; (defsubst ac-clang--get-column-bytes ()
;;   (1+ (length (encode-coding-string (buffer-substring-no-properties (line-beginning-position) (point)) 'binary))))

(defsubst ac-clang--column-number-at-pos (point)
  (save-excursion
    (goto-char point)
    (1+ (length (encode-coding-string (buffer-substring-no-properties (line-beginning-position) point) 'binary)))))


(defsubst ac-clang--get-buffer-bytes ()
  (1- (position-bytes (point-max))))


;; (defsubst ac-clang--create-position-string (point)
;;   (save-excursion
;;     (goto-char point)
;;     (format "line:%d\ncolumn:%d\n" (line-number-at-pos) (ac-clang--get-column-bytes))))

;; (defsubst ac-clang--create-position-property (packet point)
;;   (save-excursion
;;     (goto-char point)
;;     (ac-clang--add-to-command-context packet :Line (line-number-at-pos))
;;     (ac-clang--add-to-command-context packet :Column (ac-clang--get-column-bytes))))



(defmacro ac-clang--with-widening (&rest body)
  (declare (indent 0) (debug t))
  `(save-restriction
     (widen)
     (progn ,@body)))


;; (defmacro ac-clang--with-running-server (&rest body)
;;   (declare (indent 0) (debug t))
;;   (when (eq (process-status ac-clang--server-process) 'run)
;;     `(progn ,@body)))



;;;
;;; command packet utilities for json
;;;

(defvar ac-clang--ipc-request-id 0)

;; (defmacro create-command-context ()
;;   `(list :RequestId ,(setq ac-clang--ipc-request-id (1+ ac-clang--ipc-request-id))))

;; (defmacro create-command-context ()
;;   '(list :RequestId (setq ac-clang--ipc-request-id (1+ ac-clang--ipc-request-id))))

(defsubst ac-clang--create-command-context ()
  `(:RequestId ,(setq ac-clang--ipc-request-id (1+ ac-clang--ipc-request-id))))

(defmacro ac-clang--add-to-command-context (packet property value)
  `(setq ,packet (plist-put ,packet ,property ,value)))

;; (defmacro ac-clang--send-command-packet (packet)
;;   `(let ((json-object-type 'plist)
;;          (json-object (json-encode ,packet)))
;;      (ac-clang--process-send-string json-object)))

(defsubst ac-clang--send-command-packet (context)
  ;; (let ((json-object-type 'plist)
  ;;       (json-object (json-encode packet)))
  ;;   (ac-clang--process-send-string json-object)))
  (let* ((packet-object (funcall ac-clang--packet-encoder context))
         (packet-size (length packet-object))
         (send-object (concat (format "PacketSize:%d\n" packet-size) packet-object)))
    (ac-clang--process-send-string send-object)))


;; immediate create and send
(defsubst ac-clang--send-command (&rest command-plist)
  (let ((context (append (ac-clang--create-command-context) command-plist))
        (ac-clang-debug-log-buffer-p t)) ; for debug
    (ac-clang--send-command-packet context)))



;;;
;;; transaction command functions for IPC
;;;

(defvar ac-clang--command-hash (make-hash-table :test #'eq))
(defvar ac-clang--command-limit 10)


(defsubst ac-clang--request-command (sender-function receive-buffer receiver-function args)
  (if (< (ac-clang--count-command) ac-clang--command-limit)
      (progn
        (when (and receive-buffer receiver-function)
          (ac-clang--regist-command (1+ ac-clang--ipc-request-id) `(:buffer ,receive-buffer :receiver ,receiver-function :sender ,sender-function :args ,args)))
        (funcall sender-function args))
    (message "ac-clang : The number of requests of the command queue reached the limit.")
    ;; This is recovery logic.
    (when ac-clang-server-automatic-recovery-p
      (ac-clang--clear-command)
      ;; Send message
      (ac-clang-get-server-specification)
      ;; Process response wait(as with thread preemption point)
      (sleep-for 0.1)
      ;; When process response is not received, I suppose that server became to deadlock.
      (if (= (ac-clang--count-command) 0)
          (message "ac-clang : clear server commands.")
        (ac-clang-reboot-server)))))


(defsubst ac-clang--regist-command (request-id command)
  (puthash request-id command ac-clang--command-hash))


(defsubst ac-clang--unregist-command (request-id)
  (remhash request-id ac-clang--command-hash))


(defsubst ac-clang--count-command ()
  (hash-table-count ac-clang--command-hash))


(defsubst ac-clang--query-command (request-id)
  (gethash request-id ac-clang--command-hash))


(defsubst ac-clang--clear-command ()
  (clrhash ac-clang--command-hash))



;;;
;;; sender primitive functions for IPC
;;;

(defsubst ac-clang--process-send-string (string)
  (process-send-string ac-clang--server-process string)

  (when ac-clang-debug-log-buffer-p
    (let ((log-buffer (get-buffer-create ac-clang--debug-log-buffer-name)))
      (when log-buffer
        (with-current-buffer log-buffer
          (when (and ac-clang-debug-log-buffer-size (> (buffer-size) ac-clang-debug-log-buffer-size))
            (erase-buffer))

          (goto-char (point-max))
          (pp (encode-coding-string string 'binary) log-buffer)
          (insert "\n"))))))


(defsubst ac-clang--process-send-region (start end)
  (process-send-region ac-clang--server-process start end))


(defun ac-clang--send-set-clang-parameters ()
  (ac-clang--process-send-string (format "translation_unit_flags:%s\n" ac-clang-clang-translation-unit-flags))
  (ac-clang--process-send-string (format "complete_at_flags:%s\n" ac-clang-clang-complete-at-flags))
  (ac-clang--process-send-string (format "complete_results_limit:%d\n" ac-clang-clang-complete-results-limit)))


(defun ac-clang--send-cflags ()
  ;; send message head and num_cflags
  (ac-clang--process-send-string (format "num_cflags:%d\n" (length (ac-clang--build-complete-cflags))))

  (let (cflags)
    ;; create CFLAGS strings
    (mapc
     (lambda (arg)
       (setq cflags (concat cflags (format "%s\n" arg))))
     (ac-clang--build-complete-cflags))
    ;; send cflags
    (ac-clang--process-send-string cflags)))


(defun ac-clang--send-source-code ()
  (ac-clang--with-widening
    (let ((source-buffuer (current-buffer))
          (cs (coding-system-change-eol-conversion buffer-file-coding-system 'unix)))
      (with-temp-buffer
        (set-buffer-multibyte nil)
        (let ((temp-buffer (current-buffer)))
          (with-current-buffer source-buffuer
            (decode-coding-region (point-min) (point-max) cs temp-buffer)))

        (ac-clang--process-send-string (format "source_length:%d\n" (ac-clang--get-buffer-bytes)))
        ;; (ac-clang--process-send-region (point-min) (point-max))
        (ac-clang--process-send-string (buffer-substring-no-properties (point-min) (point-max)))
        (ac-clang--process-send-string "\n\n")))))


(defun ac-clang--get-source-code ()
  (ac-clang--with-widening
    (let ((source-buffuer (current-buffer))
          (cs (coding-system-change-eol-conversion buffer-file-coding-system 'unix)))
      (with-temp-buffer
        (set-buffer-multibyte nil)
        (let ((temp-buffer (current-buffer)))
          (with-current-buffer source-buffuer
            (decode-coding-region (point-min) (point-max) cs temp-buffer)))

        ;; (ac-clang--process-send-string (format "source_length:%d\n" (ac-clang--get-buffer-bytes)))
        (buffer-substring-no-properties (point-min) (point-max))))))


;; (defun ac-clang--send-source-code ()
;;   (ac-clang--with-widening
;;     (ac-clang--process-send-string (format "source_length:%d\n" (ac-clang--get-buffer-bytes)))
;;     (ac-clang--process-send-region (point-min) (point-max))
;;     (ac-clang--process-send-string "\n\n")))


;; (defsubst ac-clang--send-command (command-type command-name &optional session-name)
;;   (let ((command (format "command_type:%s\ncommand_name:%s\n" command-type command-name)))
;;     (when session-name
;;       (setq command (concat command (format "session_name:%s\n" session-name))))
;;     (ac-clang--process-send-string command)))



;;;
;;; sender command request functions for IPC
;;;

(defun ac-clang--send-server-specification-request (&optional _args)
  (ac-clang--send-command :CommandType "Server"
                          :CommandName "GET_SPECIFICATION"))


(defun ac-clang--send-clang-parameters-request (&optional _args)
  (ac-clang--send-command :CommandType "Server"
                          :CommandName "SET_CLANG_PARAMETERS"
                          :TranslationUnitFlags ac-clang-clang-translation-unit-flags
                          :CompleteAtFlags ac-clang-clang-complete-at-flags
                          :CompleteResultsLimit ac-clang-clang-complete-results-limit))


(defun ac-clang--send-create-session-request (&optional _args)
  (ac-clang--with-widening
    (ac-clang--send-command :CommandType "Server"
                            :CommandName "CREATE_SESSION"
                            :SessionName ac-clang--session-name
                            :CFLAGS (ac-clang--build-complete-cflags)
                            :SourceCode (ac-clang--get-source-code))))


(defun ac-clang--send-delete-session-request (&optional _args)
  (ac-clang--send-command :CommandType "Server"
                          :CommandName "DELETE_SESSION"
                          :SessionName ac-clang--session-name))


(defun ac-clang--send-reset-server-request (&optional _args)
  (ac-clang--send-command :CommandType "Server"
                          :CommandName "RESET"))


(defun ac-clang--send-shutdown-request (&optional _args)
  (when (eq (process-status ac-clang--server-process) 'run)
    (ac-clang--send-command :CommandType "Server"
                            :CommandName "SHUTDOWN")))


(defun ac-clang--send-suspend-request (&optional _args)
  (ac-clang--send-command :CommandType "Session"
                          :CommandName "SUSPEND"
                          :SessionName ac-clang--session-name))


(defun ac-clang--send-resume-request (&optional _args)
  (ac-clang--send-command :CommandType "Session"
                          :CommandName "RESUME"
                          :SessionName ac-clang--session-name))


(defun ac-clang--send-cflags-request (&optional _args)
  (if (listp ac-clang-cflags)
      (ac-clang--with-widening
        (ac-clang--send-command :CommandType "Session"
                                :CommandName "SET_CFLAGS"
                                :SessionName ac-clang--session-name
                                :CFLAGS (ac-clang--build-complete-cflags)
                                :SourceCode (ac-clang--get-source-code)))
    (message "`ac-clang-cflags' should be a list of strings")))


(defun ac-clang--send-reparse-request (&optional _args)
  (ac-clang--with-widening
    (ac-clang--send-command :CommandType "Session"
                            :CommandName "REPARSE"
                            :SessionName ac-clang--session-name
                            :SourceCode (ac-clang--get-source-code))))


(defun ac-clang--send-completion-request (&optional args)
  (ac-clang--with-widening
    (ac-clang--send-command :CommandType "Session"
                            :CommandName "COMPLETION"
                            :SessionName ac-clang--session-name
                            :Line (line-number-at-pos (plist-get args :start-point))
                            :Column (ac-clang--column-number-at-pos (plist-get args :start-point))
                            :SourceCode (ac-clang--get-source-code))))


(defun ac-clang--send-diagnostics-request (&optional _args)
  (ac-clang--with-widening
    (ac-clang--send-command :CommandType "Session"
                            :CommandName "SYNTAXCHECK"
                            :SessionName ac-clang--session-name
                            :SourceCode (ac-clang--get-source-code))))


(defun ac-clang--send-inclusion-request (&optional _args)
  (ac-clang--with-widening
    (ac-clang--send-command :CommandType "Session"
                            :CommandName "INCLUSION"
                            :SessionName ac-clang--session-name
                            :Line (line-number-at-pos (point))
                            :Column (ac-clang--column-number-at-pos (point))
                            :SourceCode (ac-clang--get-source-code))))


(defun ac-clang--send-definition-request (&optional _args)
  (ac-clang--with-widening
    (ac-clang--send-command :CommandType "Session"
                            :CommandName "DEFINITION"
                            :SessionName ac-clang--session-name
                            :Line (line-number-at-pos (point))
                            :Column (ac-clang--column-number-at-pos (point))
                            :SourceCode (ac-clang--get-source-code))))


(defun ac-clang--send-declaration-request (&optional _args)
  (ac-clang--with-widening
    (ac-clang--send-command :CommandType "Session"
                            :CommandName "DECLARATION"
                            :SessionName ac-clang--session-name
                            :Line (line-number-at-pos (point))
                            :Column (ac-clang--column-number-at-pos (point))
                            :SourceCode (ac-clang--get-source-code))))


(defun ac-clang--send-smart-jump-request (&optional _args)
  (ac-clang--with-widening
    (ac-clang--send-command :CommandType "Session"
                            :CommandName "SMARTJUMP"
                            :SessionName ac-clang--session-name
                            :Line (line-number-at-pos (point))
                            :Column (ac-clang--column-number-at-pos (point))
                            :SourceCode (ac-clang--get-source-code))))




;;;
;;; Receive clang-server responses common filter (receive response by command)
;;;


(defvar ac-clang--command-result-data nil)


(defsubst ac-clang--s-expression-packet-encoder (data)
  (pp-to-string data))

(defsubst ac-clang--s-expression-packet-decoder (data)
  (read data))


(defsubst ac-clang--json-packet-encoder (data)
  (let* ((json-object-type 'plist))
    (json-encode data)))

(defsubst ac-clang--json-packet-decoder (data)
  (let* ((json-object-type 'plist))
    ;; (1- (point-max)) is exclude packet termination character.
    (json-read-from-string data)))



(defvar ac-clang--packet-encoder #'ac-clang--json-packet-encoder)
(defvar ac-clang--packet-decoder #'ac-clang--json-packet-decoder)



(defun ac-clang--process-filter (process output)
  ;; IPC packet receive phase.
  (setq ac-clang--status 'receive)

  (let ((receive-buffer (process-buffer process))
        (receive-buffer-marker (process-mark process)))

    ;; check the receive buffer allocation
    (unless receive-buffer
      (when (setq receive-buffer (get-buffer-create ac-clang--process-buffer-name))
        (set-process-buffer process receive-buffer)
        (with-current-buffer receive-buffer
          (set-marker receive-buffer-marker (point-min-marker)))))

    ;; Output of the server is appended to receive buffer.
    (when receive-buffer
      (with-current-buffer receive-buffer
        (save-excursion
          ;; Insert the text, advancing the process marker.
          (goto-char receive-buffer-marker)
          (insert output)
          (set-marker receive-buffer-marker (point)))
        (goto-char receive-buffer-marker)))

    ;; Check the server response termination.('$' is packet termination character.)
    (when (string= (substring output -1 nil) "$")
      (when (> (ac-clang--count-command) 0)
        ;; IPC packet decode phase.
        (setq ac-clang--status 'transaction)
        (setq ac-clang--command-result-data (ac-clang--decode-received-packet receive-buffer))

        (let* ((request-id (plist-get ac-clang--command-result-data :RequestId))
               (command-context (ac-clang--query-command request-id)))

          (when command-context
            ;; IPC-Command execution phase.
            (ac-clang--unregist-command request-id)

            ;; setup command from context
            (let ((command-buffer (plist-get command-context :buffer))
                  (command-receiver (plist-get command-context :receiver))
                  (command-args (plist-get command-context :args)))

              ;; execute IPC-Command receiver.
              (unless (ignore-errors
                        (funcall command-receiver ac-clang--command-result-data command-args)
                        t)
                (message "ac-clang : receiver function error!"))
              ;; clear current context.
              ;; (setq ac-clang--command-result-data nil)
              ))))

      ;; clear receive-buffer for next packet.
      (with-current-buffer receive-buffer
        (erase-buffer))
      (setq ac-clang--status 'idle))))


(defun ac-clang--decode-received-packet (buffer)
  "Result value is property-list(s-expression) that converted from packet(json)."
  (with-current-buffer buffer
    ;; (1- (point-max)) is exclude packet termination character.
    (funcall ac-clang--packet-decoder (buffer-substring-no-properties (point-min) (1- (point-max))))))



;;;
;;; receive clang-server responses. 
;;; build completion candidates and fire auto-complete.
;;;

(defun ac-clang--build-completion-candidates (data start-word)
  ;; (message "ac-clang--build-completion-candidates")
  (let* ((results (plist-get data :Results))
         (pattern (regexp-quote start-word))
         candidates
         candidate
         declaration
         (prev-candidate ""))

    (mapc #'(lambda (element)
              (let ((name (plist-get element :Name))
                    (prototype (plist-get element :Prototype))
                    (brief (plist-get element :BriefComment)))
                (when (string-match-p pattern name)
                  (setq candidate name
                        declaration prototype)

                  (if (string= candidate prev-candidate)
                      (progn
                        (when declaration
                          (setq candidate (propertize candidate 'ac-clang--detail (concat (get-text-property 0 'ac-clang--detail (car candidates)) "\n" declaration)))
                          (setf (car candidates) candidate)))
                    (setq prev-candidate candidate)
                    (when declaration
                      (setq candidate (propertize candidate 'ac-clang--detail declaration)))
                    (push candidate candidates)))))
          results)
    candidates))


(defun ac-clang--build-completion-candidates-org (buffer start-word)
  (with-current-buffer buffer
    (goto-char (point-min))
    ;; (message "ac-clang--build-completion-candidates")
    (let ((pattern (format ac-clang--completion-pattern (regexp-quote start-word)))
          candidates
          candidate
          declaration
          (prev-candidate ""))
      (while (re-search-forward pattern nil t)
        (setq candidate (match-string-no-properties 1))
        (unless (string= "Pattern" candidate)
          (setq declaration (match-string-no-properties 2))

          (if (string= candidate prev-candidate)
              (progn
                (when declaration
                  (setq candidate (propertize candidate 'ac-clang--detail (concat (get-text-property 0 'ac-clang--detail (car candidates)) "\n" declaration)))
                  (setf (car candidates) candidate)))
            (setq prev-candidate candidate)
            (when declaration
              (setq candidate (propertize candidate 'ac-clang--detail declaration)))
            (push candidate candidates))))
      candidates)))


(defun ac-clang--receive-completion (data args)
  (setq ac-clang--candidates (ac-clang--build-completion-candidates data (plist-get args :start-word)))
  (setq ac-clang--start-point (plist-get args :start-point))

  ;; (setq ac-show-menu t)
  ;; (ac-start :force-init t)
  ;; (ac-update))
  (ac-complete-clang-async))



(defun ac-clang--get-autotrigger-start-point (&optional point)
  (unless point
    (setq point (point)))
  (let ((c (char-before point)))
    (when (or 
           ;; '.'
           (eq ?. c)
           ;; '->'
           (and (eq ?> c)
                (eq ?- (char-before (1- point))))
           ;; '::'
           (and (eq ?: c)
                (eq ?: (char-before (1- point)))))
      point)))


(defun ac-clang--get-manualtrigger-start-point ()
  (let* ((symbol-point (ac-prefix-symbol))
         (point (or symbol-point (point)))
         (c (char-before point)))
    (when (or 
           (ac-clang--get-autotrigger-start-point point)
           ;; ' ' for manual completion
           (eq ?\s c))
      point)))


(defsubst ac-clang--async-completion (start-point)
  (when start-point
    (ac-clang--request-command
     'ac-clang--send-completion-request
     ac-clang--completion-buffer-name
     'ac-clang--receive-completion
     (list :start-word (buffer-substring-no-properties start-point (point)) :start-point start-point))))


(defun ac-clang-async-autocomplete-autotrigger ()
  (interactive)

  (self-insert-command 1)
  (when ac-clang-async-autocompletion-automatically-p
    (ac-clang--async-completion (ac-clang--get-autotrigger-start-point))))


(defun ac-clang-async-autocomplete-manualtrigger ()
  (interactive)

  (ac-clang--async-completion (ac-clang--get-manualtrigger-start-point)))




;;;
;;; auto-complete ac-source build functions
;;;

(defsubst ac-clang--candidates ()
  ac-clang--candidates)


(defsubst ac-clang--prefix ()
  ac-clang--start-point)


(defsubst ac-clang--clean-document (s)
  (when s
    (setq s (replace-regexp-in-string "<#\\|#>\\|\\[#" "" s))
    (setq s (replace-regexp-in-string "#\\]" " " s)))
  s)


(defsubst ac-clang--in-string/comment ()
  "Return non-nil if point is in a literal (a comment or string)."
  (nth 8 (syntax-ppss)))


(defun ac-clang--action ()
  (interactive)

  ;; (ac-last-quick-help)
  (let* ((func-name (regexp-quote (substring-no-properties (cdr ac-last-completion))))
         (c/c++-pattern (format "\\(?:^.*%s\\)\\([<(].*[>)]\\)" func-name))
         (objc-pattern (format "\\(?:^.*%s\\)\\(:.*\\)" func-name))
         (detail (get-text-property 0 'ac-clang--detail (cdr ac-last-completion)))
         (help (ac-clang--clean-document detail))
         (declarations (split-string detail "\n"))
         args
         (ret-t "")
         ret-f
         candidates)

    ;; parse function or method overload declarations
    (cl-dolist (declaration declarations)
      ;; function result type
      (when (string-match "\\[#\\(.*\\)#\\]" declaration)
        (setq ret-t (match-string 1 declaration)))
      ;; remove result type
      (setq declaration (replace-regexp-in-string "\\[#.*?#\\]" "" declaration))

      ;; (message (format "comp--action: func-name=%s, detail=%s" func-name detail))

      ;; parse arguments
      (cond (;; C/C++ standard argument
             (string-match c/c++-pattern declaration)
             (setq args (match-string 1 declaration))
             (push (propertize (ac-clang--clean-document args) 'ac-clang--detail ret-t 'ac-clang--args args) candidates)
             ;; default argument
             (when (string-match "\{#" args)
               (setq args (replace-regexp-in-string "\{#.*#\}" "" args))
               (push (propertize (ac-clang--clean-document args) 'ac-clang--detail ret-t 'ac-clang--args args) candidates))
             ;; variadic argument
             (when (string-match ", \\.\\.\\." args)
               (setq args (replace-regexp-in-string ", \\.\\.\\." "" args))
               (push (propertize (ac-clang--clean-document args) 'ac-clang--detail ret-t 'ac-clang--args args) candidates)))

            (;; check whether it is a function ptr
             (string-match "^\\([^(]*\\)(\\*)\\((.*)\\)" ret-t)
             (setq ret-f (match-string 1 ret-t)
                   args (match-string 2 ret-t))
             (push (propertize args 'ac-clang--detail ret-f 'ac-clang--args "") candidates)
             ;; variadic argument
             (when (string-match ", \\.\\.\\." args)
               (setq args (replace-regexp-in-string ", \\.\\.\\." "" args))
               (push (propertize args 'ac-clang--detail ret-f 'ac-clang--args "") candidates)))

            (;; Objective-C/C++ argument
             (string-match objc-pattern declaration)
             (setq args (match-string 1 declaration))
             (push (propertize (ac-clang--clean-document args) 'ac-clang--detail ret-t 'ac-clang--args args) candidates))))

    (cond (candidates
           (setq candidates (delete-dups candidates))
           (setq candidates (nreverse candidates))
           (setq ac-clang--template-candidates candidates)
           (setq ac-clang--template-start-point (point))
           (ac-complete-clang-template)

           (unless (cdr candidates) ;; unless length > 1
             (message (replace-regexp-in-string "\n" "   ;    " help))))
          (t
           (message (replace-regexp-in-string "\n" "   ;    " help))))))


(defun ac-clang--document (item)
  (if (stringp item)
      (let (s)
        (setq s (get-text-property 0 'ac-clang--detail item))
        ;; (message (format "clang--document: item=%s, s=%s" item s))
        (ac-clang--clean-document s)))
  ;; (popup-item-property item 'ac-clang--detail)
  ;; nil
  )



(ac-define-source clang-async
  '((candidates     . ac-clang--candidates)
    (candidate-face . ac-clang-candidate-face)
    (selection-face . ac-clang-selection-face)
    (prefix         . ac-clang--prefix)
    (requires       . 0)
    (action         . ac-clang--action)
    (document       . ac-clang--document)
    (cache)
    (symbol         . "c")))



(defun ac-clang--same-count-in-string (c1 c2 s)
  (let ((count 0)
        (cur 0)
        (end (length s))
        c)
    (while (< cur end)
      (setq c (aref s cur))
      (cond ((eq c1 c)
             (setq count (1+ count)))
            ((eq c2 c)
             (setq count (1- count))))
      (setq cur (1+ cur)))
    (= count 0)))


(defun ac-clang--split-args (s)
  (let ((sl (split-string s ", *")))
    (cond ((string-match "<\\|(" s)
           (let (res
                 (pre "")
                 subs)
             (while sl
               (setq subs (pop sl))
               (unless (string= pre "")
                 (setq subs (concat pre ", " subs))
                 (setq pre ""))
               (cond ((and (ac-clang--same-count-in-string ?\< ?\> subs)
                           (ac-clang--same-count-in-string ?\( ?\) subs))
                      ;; (cond ((ac-clang--same-count-in-string ?\< ?\> subs)
                      (push subs res))
                     (t
                      (setq pre subs))))
             (nreverse res)))
          (t
           sl))))


(defsubst ac-clang--template-candidates ()
  ac-clang--template-candidates)


(defsubst ac-clang--template-prefix ()
  ac-clang--template-start-point)


(defun ac-clang--template-action ()
  (interactive)

  (when ac-clang--template-start-point
    (let ((point (point))
          sl 
          (snp "")
          (s (get-text-property 0 'ac-clang--args (cdr ac-last-completion))))
      ;; (message (format "org=%s" s))
      (cond (;; function ptr call
             (string= s "")
             (setq s (cdr ac-last-completion))
             (setq s (replace-regexp-in-string "^(\\|)$" "" s))
             (setq sl (ac-clang--split-args s))
             (cl-dolist (arg sl)
               (setq snp (concat snp ", ${" arg "}")))
             ;; (message (format "t0:s1=%s, s=%s, snp=%s" s1 s snp))
             (yas-expand-snippet (concat "("  (substring snp 2) ")") ac-clang--template-start-point point))
            (;; function args
             t
             (unless (string= s "()")
               (setq s (replace-regexp-in-string "{#" "" s))
               (setq s (replace-regexp-in-string "#}" "" s))
               (setq s (replace-regexp-in-string "<#" "${" s))
               (setq s (replace-regexp-in-string "#>" "}" s))
               (setq s (replace-regexp-in-string ", \\.\\.\\." "}, ${..." s))
               ;; (message (format "t1:s=%s" s))
               (yas-expand-snippet s ac-clang--template-start-point point)))))))


;; This source shall only be used internally.
(ac-define-source clang-template
  '((candidates . ac-clang--template-candidates)
    (prefix     . ac-clang--template-prefix)
    (requires   . 0)
    (action     . ac-clang--template-action)
    (document   . ac-clang--document)
    (cache)
    (symbol     . "t")))





;;;
;;; receive clang-server responses. 
;;; syntax checking with flymake
;;;

(defun ac-clang--receive-diagnostics (data _args)
  (let* ((results (plist-get data :Results))
         (diagnostics (plist-get results :Diagnostics)))
    (flymake-log 3 "received data")
    (flymake-parse-output-and-residual diagnostics))

  (flymake-parse-residual)
  (setq flymake-err-info flymake-new-err-info)
  (setq flymake-new-err-info nil)
  (setq flymake-err-info (flymake-fix-line-numbers flymake-err-info 1 (count-lines (point-min) (point-max))))
  (flymake-delete-own-overlays)
  (flymake-highlight-err-lines flymake-err-info))


(defun ac-clang-diagnostics ()
  (interactive)

  (if ac-clang--suspend-p
      (ac-clang-resume)
    (ac-clang-activate))

  (ac-clang--request-command 'ac-clang--send-diagnostics-request ac-clang--diagnostics-buffer-name 'ac-clang--receive-diagnostics nil))




;;;
;;; receive clang-server responses. 
;;; jump declaration/definition/smart-jump
;;;

(defun ac-clang--receive-jump (data _arg)
  (let* ((results (plist-get data :Results))
         (filename (plist-get results :Path))
         (line (plist-get results :Line))
         (column (1- (plist-get results :Column)))
         (new-loc `(,filename ,line ,column))
         (current-loc (list (buffer-file-name) (line-number-at-pos) (current-column))))
    (when (not (equal current-loc new-loc))
      (push current-loc ac-clang--jump-stack)
      (ac-clang--jump new-loc))))


(defun ac-clang--jump (location)
  (let* ((filename (pop location))
         (line (pop location))
         (column (pop location)))
    (find-file filename)
    (goto-char (point-min))
    (forward-line (1- line))
    (move-to-column column)))


(defun ac-clang-jump-back ()
  (interactive)

  (when ac-clang--jump-stack
    (ac-clang--jump (pop ac-clang--jump-stack))))


(defun ac-clang-jump-inclusion ()
  (interactive)

  (if ac-clang--suspend-p
      (ac-clang-resume)
    (ac-clang-activate))

  (ac-clang--request-command 'ac-clang--send-inclusion-request ac-clang--process-buffer-name 'ac-clang--receive-jump nil))


(defun ac-clang-jump-definition ()
  (interactive)

  (if ac-clang--suspend-p
      (ac-clang-resume)
    (ac-clang-activate))

  (ac-clang--request-command 'ac-clang--send-definition-request ac-clang--process-buffer-name 'ac-clang--receive-jump nil))


(defun ac-clang-jump-declaration ()
  (interactive)

  (if ac-clang--suspend-p
      (ac-clang-resume)
    (ac-clang-activate))

  (ac-clang--request-command 'ac-clang--send-declaration-request ac-clang--process-buffer-name 'ac-clang--receive-jump nil))


(defun ac-clang-jump-smart ()
  (interactive)

  (if ac-clang--suspend-p
      (ac-clang-resume)
    (ac-clang-activate))

  (ac-clang--request-command 'ac-clang--send-smart-jump-request ac-clang--process-buffer-name 'ac-clang--receive-jump nil))




;;;
;;; sender function for IPC
;;;

(defun ac-clang-get-server-specification ()
  (interactive)

  (when ac-clang--server-process
    (ac-clang--request-command 'ac-clang--send-server-specification-request ac-clang--process-buffer-name #'(lambda (_data _args)) nil)))



;;;
;;; The session control functions
;;;

(defun ac-clang-activate ()
  (interactive)

  (remove-hook 'first-change-hook 'ac-clang-activate t)

  (unless ac-clang--activate-p
    ;; (if ac-clang--activate-buffers
    ;;  (ac-clang-update-cflags)
    ;;   (ac-clang-initialize))

    (setq ac-clang--activate-p t)
    (setq ac-clang--session-name (buffer-file-name))
    (setq ac-clang--suspend-p nil)
    (setq ac-clang--ac-sources-backup ac-sources)
    (setq ac-sources '(ac-source-clang-async))
    (push (current-buffer) ac-clang--activate-buffers)

    (ac-clang--send-create-session-request)

    (local-set-key (kbd ".") 'ac-clang-async-autocomplete-autotrigger)
    (local-set-key (kbd ">") 'ac-clang-async-autocomplete-autotrigger)
    (local-set-key (kbd ":") 'ac-clang-async-autocomplete-autotrigger)
    (local-set-key (kbd ac-clang-async-autocompletion-manualtrigger-key) 'ac-clang-async-autocomplete-manualtrigger)

    (add-hook 'before-save-hook 'ac-clang-suspend nil t)
    ;; (add-hook 'after-save-hook 'ac-clang-deactivate nil t)
    ;; (add-hook 'first-change-hook 'ac-clang-activate nil t)
    ;; (add-hook 'before-save-hook 'ac-clang-reparse-buffer nil t)
    ;; (add-hook 'after-save-hook 'ac-clang-reparse-buffer nil t)
    (add-hook 'before-revert-hook 'ac-clang-deactivate nil t)
    (add-hook 'kill-buffer-hook 'ac-clang-deactivate nil t)))


(defun ac-clang-deactivate ()
  (interactive)

  (when ac-clang--activate-p
    (remove-hook 'before-save-hook 'ac-clang-suspend t)
    (remove-hook 'first-change-hook 'ac-clang-resume t)
    ;; (remove-hook 'before-save-hook 'ac-clang-reparse-buffer t)
    ;; (remove-hook 'after-save-hook 'ac-clang-reparse-buffer t)
    (remove-hook 'before-revert-hook 'ac-clang-deactivate t)
    (remove-hook 'kill-buffer-hook 'ac-clang-deactivate t)

    (ac-clang--send-delete-session-request)

    (pop ac-clang--activate-buffers)
    (setq ac-sources ac-clang--ac-sources-backup)
    (setq ac-clang--ac-sources-backup nil)
    (setq ac-clang--suspend-p nil)
    (setq ac-clang--session-name nil)
    (setq ac-clang--activate-p nil)

    ;; (unless ac-clang--activate-buffers
    ;;   (ac-clang-finalize))
    ))


(defun ac-clang-activate-after-modify ()
  (interactive)

  (if (buffer-modified-p)
      (ac-clang-activate)
    (add-hook 'first-change-hook 'ac-clang-activate nil t)))


(defun ac-clang-suspend ()
  (when (and ac-clang--activate-p (not ac-clang--suspend-p))
    (setq ac-clang--suspend-p t)
    (ac-clang--send-suspend-request)
    (add-hook 'first-change-hook 'ac-clang-resume nil t)))


(defun ac-clang-resume ()
  (when (and ac-clang--activate-p ac-clang--suspend-p)
    (setq ac-clang--suspend-p nil)
    (remove-hook 'first-change-hook 'ac-clang-resume t)
    (ac-clang--send-resume-request)))


(defun ac-clang-reparse-buffer ()
  (when ac-clang--server-process
    (ac-clang--send-reparse-request)))


(defun ac-clang-update-cflags ()
  (interactive)

  (when ac-clang--activate-p
    ;; (message "ac-clang-update-cflags %s" ac-clang--session-name)
    (ac-clang--send-cflags-request)))


(defun ac-clang-set-cflags ()
  "Set `ac-clang-cflags' interactively."
  (interactive)

  (setq ac-clang-cflags (split-string (read-string "New cflags: ")))
  (ac-clang-update-cflags))


(defun ac-clang-set-cflags-from-shell-command ()
  "Set `ac-clang-cflags' to a shell command's output.
  set new cflags for ac-clang from shell command output"
  (interactive)

  (setq ac-clang-cflags
        (split-string
         (shell-command-to-string
          (read-shell-command "Shell command: " nil nil
                              (and buffer-file-name
                                   (file-relative-name buffer-file-name))))))
  (ac-clang-update-cflags))


(defun ac-clang-set-prefix-header (prefix-header)
  "Set `ac-clang-prefix-header' interactively."
  (interactive
   (let ((default (car (directory-files "." t "\\([^.]h\\|[^h]\\).pch\\'" t))))
     (list
      (read-file-name (concat "Clang prefix header (currently " (or ac-clang-prefix-header "nil") "): ")
                      (when default (file-name-directory default))
                      default nil (when default (file-name-nondirectory default))))))

  (cond
   ((string-match "^[\s\t]*$" prefix-header)
    (setq ac-clang-prefix-header nil))
   (t
    (setq ac-clang-prefix-header prefix-header))))




;;;
;;; The server control functions
;;;


(defun ac-clang--clean-tmp-pch ()
  "Clean up temporary precompiled headers."

  (cl-dolist (pch-file (directory-files temporary-file-directory t "preamble-.*\\.pch$" t))
    (ignore-errors
      (delete-file pch-file)
      t)))



(defun ac-clang-launch-server ()
  (interactive)

  (when (and ac-clang--server-executable (not ac-clang--server-process))
    (let ((process-connection-type nil)
          (coding-system-for-write 'binary))
      (setq ac-clang--server-process
            (apply 'start-process
                   ac-clang--process-name ac-clang--process-buffer-name
                   ac-clang--server-executable (ac-clang--build-server-launch-options))))

    (if ac-clang--server-process
        (progn
          (setq ac-clang--status 'idle)
          (ac-clang--clear-command)

          (set-process-coding-system ac-clang--server-process
                                     (coding-system-change-eol-conversion buffer-file-coding-system nil)
                                     'binary)
          (set-process-filter ac-clang--server-process 'ac-clang--process-filter)
          (set-process-query-on-exit-flag ac-clang--server-process nil)

          (ac-clang--send-clang-parameters-request)
          t)
      (display-warning 'ac-clang "clang-server launch failed.")
      nil)))


(defun ac-clang-shutdown-server ()
  (interactive)

  (when ac-clang--server-process
    (ac-clang--send-shutdown-request)

    (setq ac-clang--status 'shutdown)

    (setq ac-clang--server-process nil)
    t))


(defun ac-clang-update-clang-parameters ()
  (interactive)

  (when ac-clang--server-process
    (ac-clang--send-clang-parameters-request)
    t))


(defun ac-clang-reset-server ()
  (interactive)

  (when ac-clang--server-process
    (cl-dolist (buffer ac-clang--activate-buffers)
      (with-current-buffer buffer 
        (ac-clang-deactivate)))
    (ac-clang--send-reset-server-request)))


(cl-defun ac-clang-reboot-server ()
  (interactive)

  (let ((buffers ac-clang--activate-buffers))
    (ac-clang-reset-server)

    (unless (ac-clang-shutdown-server)
      (message "ac-clang : reboot server failed.")
      (cl-return-from ac-clang-reset-server nil))

    (unless (ac-clang-launch-server)
      (message "ac-clang : reboot server failed.")
      (cl-return-from ac-clang-reset-server nil))

    (cl-dolist (buffer buffers)
      (with-current-buffer buffer
        (ac-clang-activate))))

  (message "ac-clang : reboot server success.")
  t)




(defun ac-clang-initialize ()
  (interactive)

  ;; server binary decide
  (unless ac-clang--server-executable
    (setq ac-clang--server-executable (executable-find (or (plist-get ac-clang--server-binaries ac-clang-server-type) ""))))

  ;; check obsolete
  (unless ac-clang--server-executable
    (when (setq ac-clang--server-executable (executable-find (or (plist-get ac-clang--server-obsolete-binaries ac-clang-server-type) "")))
      (display-warning 'ac-clang "The clang-server which you are using is obsolete. please replace to the new binary.")))

  ;; (message "ac-clang-initialize")
  (if ac-clang--server-executable
      (when (ac-clang-launch-server)
        ;; Optional keybindings
        (define-key ac-mode-map (kbd "M-.") 'ac-clang-jump-smart)
        (define-key ac-mode-map (kbd "M-,") 'ac-clang-jump-back)
        ;; (define-key ac-mode-map (kbd "C-c `") 'ac-clang-diagnostics)) 

        (add-hook 'kill-emacs-hook 'ac-clang-finalize)

        (when (and (eq system-type 'windows-nt) (boundp 'w32-pipe-read-delay) (> w32-pipe-read-delay 0))
          (display-warning 'ac-clang "Please set the appropriate value for `w32-pipe-read-delay'. Because a pipe delay value is large value. Ideal value is 0. see help of `w32-pipe-read-delay'."))

        t)
    (display-warning 'ac-clang "clang-server binary not found.")
    nil))


(defun ac-clang-finalize ()
  (interactive)

  ;; (message "ac-clang-finalize")
  (when (ac-clang-shutdown-server)
    (define-key ac-mode-map (kbd "M-.") nil)
    (define-key ac-mode-map (kbd "M-,") nil)

    (setq ac-clang--server-executable nil)

    (when ac-clang-tmp-pch-automatic-cleanup-p
      (ac-clang--clean-tmp-pch))

    t))





(provide 'ac-clang)

;; Local Variables:
;; coding: utf-8
;; indent-tabs-mode: nil
;; End:

;;; ac-clang.el ends here
