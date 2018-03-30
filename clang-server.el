;;; clang-server.el --- Auto Completion source by libclang for GNU Emacs -*- lexical-binding: t; -*-

;;; last updated : 2018/03/30.20:07:05

;; Copyright (C) 2010       Brian Jiang
;; Copyright (C) 2012       Taylan Ulrich Bayirli/Kammer
;; Copyright (C) 2013       Golevka
;; Copyright (C) 2013-2018  yaruopooner
;; 
;; This file is part of ac-clang.


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

;;; Code:



(eval-when-compile (require 'cl-lib))
(eval-when-compile (require 'pp))
(eval-when-compile (require 'json))


(defconst ac-clang-version "2.1.0")




;;;
;;; for Server vars
;;;

;; clang-server binary type
(defvar ac-clang-server-type 'release
  "clang-server binary type
`release'  : release build version
`debug'    : debug build version (server develop only)
`test'     : feature test version (server develop only)
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

(defvar ac-clang-server-input-data-type 's-expression
  "The server receive(STDIN) data type.
`s-expression' : s-expression format (default)
`json'         : json format
")

(defvar ac-clang-server-output-data-type 's-expression
  "The server send(STDOUT) data type.
`s-expression' : s-expression format (default)
`json'         : json format
")

(defvar ac-clang-server-logfile nil
  "IPC records output file.(for debug)")


;; server binaries property list
(defconst ac-clang--server-binaries '(release "clang-server"
                                      debug   "clang-server-debug"
                                      test    "clang-server-test"))


;; server binary require version
(defconst ac-clang--server-require-version '(2 0 0)
  "(MAJOR MINOR MAINTENANCE)")


;; server process details
(defcustom ac-clang--server-executable nil
  "Location of clang-server executable."
  :group 'auto-complete
  :type 'file)


(defconst ac-clang--process-name "Clang-Server")

(defconst ac-clang--process-buffer-name "*Clang-Server*")

(defvar ac-clang--server-process nil)
(defvar ac-clang--status 'idle
  "clang-server status
`idle'          : job is nothing
`receive'       : receiving command sent result
`transaction'   : transaction execute to received command result
`shutdown'      : shutdown server
  ")


(defvar ac-clang--activate-buffers nil)


;; clang-server behaviors
(defvar ac-clang-clang-translation-unit-flags "CXTranslationUnit_DetailedPreprocessingRecord|CXTranslationUnit_Incomplete|CXTranslationUnit_PrecompiledPreamble|CXTranslationUnit_CacheCompletionResults|CXTranslationUnit_IncludeBriefCommentsInCodeCompletion|CXTranslationUnit_CreatePreambleOnFirstParse"
  "CXTranslationUnit Flags. 
for Server behavior.
The value sets flag-name strings or flag-name combined strings.
Separator is `|'.
`CXTranslationUnit_DetailedPreprocessingRecord'            : Required if you want jump to macro declaration, inclusion-file.
`CXTranslationUnit_Incomplete'                             : for PCH
`CXTranslationUnit_PrecompiledPreamble'                    : Increase completion performance.
`CXTranslationUnit_CacheCompletionResults'                 : Increase completion performance.
`CXTranslationUnit_ForSerialization'                       :  
`CXTranslationUnit_CXXChainedPCH'                          :  
`CXTranslationUnit_SkipFunctionBodies'                     :  
`CXTranslationUnit_IncludeBriefCommentsInCodeCompletion'   : Required if you want to brief-comment of completion.
`CXTranslationUnit_CreatePreambleOnFirstParse'             : Increase completion performance.
`CXTranslationUnit_KeepGoing'                              : 
`CXTranslationUnit_SingleFileParse'                        : 
")

(defvar ac-clang-clang-complete-at-flags "CXCodeComplete_IncludeMacros|CXCodeComplete_IncludeCodePatterns|CXCodeComplete_IncludeBriefComments"
  "CXCodeComplete Flags. 
for Server behavior.
The value sets flag-name strings or flag-name combined strings.
Separator is `|'.
`CXCodeComplete_IncludeMacros'                             :
`CXCodeComplete_IncludeCodePatterns'                       :
`CXCodeComplete_IncludeBriefComments'                      : You need to set `CXTranslationUnit_IncludeBriefCommentsInCodeCompletion' in ac-clang-clang-translation-unit-flags.

This flags is same as below clang command line options. 
-code-completion-macros
-code-completion-patterns
-code-completion-brief-comments
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
;;; for Session vars
;;;

(defvar-local ac-clang--activate-p nil)

(defvar-local ac-clang--session-name nil)


;; CFLAGS build behaviors
(defvar-local ac-clang-language-option-function nil
  "Function to return the language type for option -x.")

(defvar-local ac-clang-prefix-header nil
  "The prefix header to pass to the Clang executable.")


;; clang-server session behavior
(defvar-local ac-clang-cflags nil
  "Extra flags to pass to the Clang executable.
This variable will typically contain include paths, e.g., (\"-I~/MyProject\" \"-I.\").")




;;;
;;; for transaction vars
;;;

(defvar ac-clang--transaction-id 0)

(defvar ac-clang--transaction-hash (make-hash-table :test #'eq))
(defvar ac-clang--transaction-limit 10)


(defvar ac-clang--command-result-data nil)
(defvar ac-clang--completion-command-result-data nil)


(defconst ac-clang--packet-encoder/decoder-infos '(s-expression
                                                   (:encoder
                                                    ac-clang--encode-s-expression-packet
                                                    :decoder
                                                    ac-clang--decode-s-expression-packet)
                                                   json
                                                   (:encoder
                                                    ac-clang--encode-json-packet
                                                    :decoder
                                                    ac-clang--decode-json-packet)))


(defvar ac-clang--packet-encoder nil
  "Specify the function to be used encoding.
Automatic set from value of ac-clang-server-input-data-type.
#'ac-clang--encode-s-expression-packet
#'ac-clang--encode-json-packet
")

(defvar ac-clang--packet-decoder nil
  "Specify the function to be used decoding.
Automatic set from value of ac-clang-server-output-data-type.
#'ac-clang--decode-s-expression-packet
#'ac-clang--decode-json-packet
")




;; transaction send packet debug
(defconst ac-clang--debug-log-buffer-name "*Clang-Log*")
(defvar ac-clang-debug-log-buffer-p nil)
(defvar ac-clang-debug-log-buffer-size (* 1024 50))


;; transaction performance profiler debug
(defvar ac-clang-debug-profiler-p nil)
(defvar ac-clang--debug-profiler-hash (make-hash-table :test #'eq))
(defvar ac-clang--debug-profiler-display-marks '((:transaction-register :packet-receive)
                                                 (:packet-receive :packet-decode)
                                                 (:packet-decode :transaction-receiver)
                                                 (:transaction-register :transaction-receiver)))




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
   (when ac-clang-server-input-data-type
     (list "--input-data" (format "%S" ac-clang-server-input-data-type)))
   (when ac-clang-server-output-data-type
     (list "--output-data" (format "%S" ac-clang-server-output-data-type)))
   (when ac-clang-server-logfile
     (list "--logfile" (format "%s" ac-clang-server-logfile)))))


;; CFLAGS builders
(defsubst ac-clang--build-language-option ()
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


;; CFLAGS builders
(defsubst ac-clang--build-complete-cflags ()
  (append '("-cc1" "-fsyntax-only")
          (list "-x" (ac-clang--build-language-option))
          ac-clang-cflags
          (when (stringp ac-clang-prefix-header)
            (list "-include-pch" (expand-file-name ac-clang-prefix-header)))))




;;;
;;; source code utilities
;;;

;; (defsubst ac-clang--get-column-bytes ()
;;   (1+ (length (encode-coding-string (buffer-substring-no-properties (line-beginning-position) (point)) 'binary))))


(defsubst ac-clang--column-number-at-pos (point)
  (save-excursion
    (goto-char point)
    (1+ (length (encode-coding-string (buffer-substring-no-properties (line-beginning-position) point) 'binary)))))


(defsubst ac-clang--get-buffer-bytes ()
  (1- (position-bytes (point-max))))


(defmacro ac-clang--with-widening (&rest body)
  (declare (indent 0) (debug t))
  `(save-restriction
     (widen)
     (progn ,@body)))


(defun ac-clang--get-source-code ()
  (ac-clang--with-widening
    (let ((source-buffuer (current-buffer))
          (cs (coding-system-change-eol-conversion buffer-file-coding-system 'unix)))
      (with-temp-buffer
        (set-buffer-multibyte nil)
        (let ((temp-buffer (current-buffer)))
          (with-current-buffer source-buffuer
            (decode-coding-region (point-min) (point-max) cs temp-buffer)))

        (buffer-substring-no-properties (point-min) (point-max))))))


;; (defmacro ac-clang--with-running-server (&rest body)
;;   (declare (indent 0) (debug t))
;;   (when (eq (process-status ac-clang--server-process) 'run)
;;     `(progn ,@body)))




;;;
;;; performance profiler functions for IPC
;;;

;; (defsubst ac-clang--mark-and-register-profiler (transaction-id mark-property)
;;   (when ac-clang-debug-profiler-p
;;     (setf (gethash transaction-id ac-clang--debug-profiler-hash) (append (gethash transaction-id ac-clang--debug-profiler-hash) `(,mark-property ,(float-time))))))

(defmacro ac-clang--mark-and-register-profiler (transaction-id mark-property)
  `(when ac-clang-debug-profiler-p
     (setf (gethash ,transaction-id ac-clang--debug-profiler-hash) (append (gethash ,transaction-id ac-clang--debug-profiler-hash) (list ,mark-property (float-time))))))


;; (defsubst ac-clang--mark-profiler (profile-plist mark-property)
;;   (when ac-clang-debug-profiler-p
;;     (set profile-plist (append (symbol-value profile-plist) `(,mark-property ,(float-time))))))

(defmacro ac-clang--mark-profiler (profile-plist mark-property)
  `(when ac-clang-debug-profiler-p
     (setq ,profile-plist (append ,profile-plist (list ,mark-property (float-time))))))


;; (defsubst ac-clang--append-profiler (transaction-id profile-plist)
;;   (when ac-clang-debug-profiler-p
;;     (setf (gethash transaction-id ac-clang--debug-profiler-hash) (append (gethash transaction-id ac-clang--debug-profiler-hash) profile-plist))))

(defmacro ac-clang--append-profiler (transaction-id profile-plist)
  `(when ac-clang-debug-profiler-p
     (setf (gethash ,transaction-id ac-clang--debug-profiler-hash) (append (gethash ,transaction-id ac-clang--debug-profiler-hash) ,profile-plist))))


(defun ac-clang--display-profiler (transaction-id)
  (when ac-clang-debug-profiler-p
    (let ((plist (gethash transaction-id ac-clang--debug-profiler-hash)))
      (when plist
        (message "ac-clang : performance profiler : transaction-id : %d" transaction-id)
        (message "ac-clang :  [ mark-begin                => mark-end                  ] elapsed-time(s)")
        (message "ac-clang : -----------------------------------------------------------------------")
        (cl-dolist (begin-end ac-clang--debug-profiler-display-marks)
          (let* ((begin (nth 0 begin-end))
                 (end (nth 1 begin-end))
                 (begin-time (plist-get plist begin))
                 (end-time (plist-get plist end)))
            (when (and begin-time end-time)
              (message "ac-clang :  [ %-25s => %-25s ] %8.3f" (symbol-name begin) (symbol-name end) (- end-time begin-time)))))))))


(defun ac-clang--display-server-profiler (profiles)
  (message "clang-server : Sampled Profiles")
  (message "clang-server :  scope-name                               : elapsed-time(ms)")
  (message "clang-server : -----------------------------------------------------------------------")
  ;; type of profiles is vector
  (mapc (lambda (profile)
          (let ((name (plist-get profile :Name))
                (elapsed-time (plist-get profile :ElapsedTime)))
            (message "clang-server :  %-40s : %8.3f" name elapsed-time)))
        profiles)
  ;; (message "ac-clang : server side profiles")
  ;; (message "%S" profiles)
  )




;;;
;;; transaction functions for IPC
;;;

(defsubst ac-clang--register-transaction (transaction)
  ;; (message "ac-clang--register-transaction : %s" transaction)
  (ac-clang--mark-and-register-profiler ac-clang--transaction-id :transaction-register)
  (puthash ac-clang--transaction-id transaction ac-clang--transaction-hash))


(defsubst ac-clang--unregister-transaction (transaction-id)
  (let ((transaction (gethash transaction-id ac-clang--transaction-hash)))
    (when transaction
      (remhash transaction-id ac-clang--transaction-hash))
    transaction))


(defsubst ac-clang--count-transaction ()
  (hash-table-count ac-clang--transaction-hash))


(defsubst ac-clang--query-transaction (transaction-id)
  (gethash transaction-id ac-clang--transaction-hash))


(defsubst ac-clang--clear-transaction ()
  (clrhash ac-clang--transaction-hash))


(defsubst ac-clang--request-transaction (sender-function receiver-function args)
  (if (< (ac-clang--count-transaction) ac-clang--transaction-limit)
      (progn
        (when receiver-function
          (ac-clang--register-transaction `(:receiver ,receiver-function :sender ,sender-function :args ,args)))
        (funcall sender-function args))

    ;; This is recovery logic.
    (message "ac-clang : The number of requests of the transaction reached the limit.")
    (when ac-clang-server-automatic-recovery-p
      (ac-clang--clear-transaction)
      ;; Send command
      (ac-clang-get-server-specification)
      ;; Process response wait(as with thread preemption point)
      (sleep-for 0.1)
      ;; When process response is not received, I suppose that server became to deadlock.
      (if (= (ac-clang--count-transaction) 0)
          (message "ac-clang : clear transaction requests.")
        (ac-clang-reboot-server)))))




;;;
;;; send primitive functions for IPC
;;;

(defsubst ac-clang--process-send-string (string)
  (when ac-clang-debug-log-buffer-p
    (let ((log-buffer (get-buffer-create ac-clang--debug-log-buffer-name)))
      (when log-buffer
        (with-current-buffer log-buffer
          (when (and ac-clang-debug-log-buffer-size (> (buffer-size) ac-clang-debug-log-buffer-size))
            (erase-buffer))

          (goto-char (point-max))
          (pp (encode-coding-string string 'binary) log-buffer)
          (insert "\n")))))

  (process-send-string ac-clang--server-process string))




;;;
;;; encoder/decoder for Packet
;;;

(defsubst ac-clang--encode-plane-text-packet (data)
  data)

(defsubst ac-clang--decode-plane-text-packet (data)
  data)


(defsubst ac-clang--encode-s-expression-packet (data)
  (format "%S" data))
  ;; (ac-clang--mark-and-register-profiler ac-clang--transaction-id :packet-encode)
  ;; (let ((pp-escape-newlines nil))
  ;;   (pp-to-string data)))

(defsubst ac-clang--decode-s-expression-packet (data)
  (read data))


(defsubst ac-clang--encode-json-packet (data)
  (let* ((json-object-type 'plist)
         (json-array-type 'vector))
    (json-encode data))
  ;; (ac-clang--mark-and-register-profiler ac-clang--transaction-id :packet-encode)
  )

(defsubst ac-clang--decode-json-packet (data)
  (let* ((json-object-type 'plist)
         (json-array-type 'vector))
    (json-read-from-string data)))




;;;
;;; command packet utilities
;;;

(defsubst ac-clang--create-command-context (command-plist)
  ;; (message "ac-clang--create-command-context : transaction-id %d" ac-clang--transaction-id)
  (let* ((header `(:RequestId ,ac-clang--transaction-id))
         (context (append header command-plist (and ac-clang-debug-profiler-p '(:IsProfile t)))))
    ;; Caution:
    ;; The reference and change of transaction-id must be completed before transmitting the packet.
    ;; The reason is that there is a possibility that references and changes to the transaction-id 
    ;; may be made in the background at the time of packet transmission, resulting in a state like a thread data race.
    (setq ac-clang--transaction-id (1+ ac-clang--transaction-id))
    context))

(defsubst ac-clang--send-command-packet (context)
  (let* ((packet-object (funcall ac-clang--packet-encoder context))
         (packet-size (length packet-object))
         (send-object (concat (format "PacketSize:%d\n" packet-size) packet-object)))
    (ac-clang--process-send-string send-object)))


;; immediate create and send
(defsubst ac-clang--send-command (&rest command-plist)
  (let ((context (ac-clang--create-command-context command-plist))
        ;; (ac-clang-debug-log-buffer-p t) ; for debug
        )
    (ac-clang--send-command-packet context)))




;;;
;;; server command sender functions for IPC
;;;

(defun ac-clang--send-server-specification-command (&optional _args)
  (ac-clang--send-command :CommandType "Server"
                          :CommandName "GET_SPECIFICATION"))


(defun ac-clang--send-clang-parameters-command (&optional _args)
  (ac-clang--send-command :CommandType "Server"
                          :CommandName "SET_CLANG_PARAMETERS"
                          :TranslationUnitFlags ac-clang-clang-translation-unit-flags
                          :CompleteAtFlags ac-clang-clang-complete-at-flags
                          :CompleteResultsLimit ac-clang-clang-complete-results-limit))


(defun ac-clang--send-create-session-command (&optional _args)
  (ac-clang--with-widening
    (ac-clang--send-command :CommandType "Server"
                            :CommandName "CREATE_SESSION"
                            :SessionName ac-clang--session-name
                            :CFLAGS (ac-clang--build-complete-cflags)
                            :SourceCode (ac-clang--get-source-code))))


(defun ac-clang--send-delete-session-command (&optional _args)
  (ac-clang--send-command :CommandType "Server"
                          :CommandName "DELETE_SESSION"
                          :SessionName ac-clang--session-name))


(defun ac-clang--send-reset-server-command (&optional _args)
  (ac-clang--send-command :CommandType "Server"
                          :CommandName "RESET"))


(defun ac-clang--send-shutdown-command (&optional _args)
  (when (eq (process-status ac-clang--server-process) 'run)
    (ac-clang--send-command :CommandType "Server"
                            :CommandName "SHUTDOWN")))


(defun ac-clang--send-suspend-command (&optional _args)
  (ac-clang--send-command :CommandType "Session"
                          :CommandName "SUSPEND"
                          :SessionName ac-clang--session-name))


(defun ac-clang--send-resume-command (&optional _args)
  (ac-clang--send-command :CommandType "Session"
                          :CommandName "RESUME"
                          :SessionName ac-clang--session-name))


(defun ac-clang--send-cflags-command (&optional _args)
  (if (listp ac-clang-cflags)
      (ac-clang--with-widening
        (ac-clang--send-command :CommandType "Session"
                                :CommandName "SET_CFLAGS"
                                :SessionName ac-clang--session-name
                                :CFLAGS (ac-clang--build-complete-cflags)
                                :SourceCode (ac-clang--get-source-code)))
    (message "`ac-clang-cflags' should be a list of strings")))


(defun ac-clang--send-reparse-command (&optional _args)
  (ac-clang--with-widening
    (ac-clang--send-command :CommandType "Session"
                            :CommandName "REPARSE"
                            :SessionName ac-clang--session-name
                            :SourceCode (ac-clang--get-source-code))))


(defun ac-clang--send-completion-command (&optional args)
  (ac-clang--with-widening
    (ac-clang--send-command :CommandType "Session"
                            :CommandName "COMPLETION"
                            :SessionName ac-clang--session-name
                            :Line (line-number-at-pos (plist-get args :start-point))
                            :Column (ac-clang--column-number-at-pos (plist-get args :start-point))
                            :SourceCode (ac-clang--get-source-code))))


(defun ac-clang--send-diagnostics-command (&optional _args)
  (ac-clang--with-widening
    (ac-clang--send-command :CommandType "Session"
                            :CommandName "SYNTAXCHECK"
                            :SessionName ac-clang--session-name
                            :SourceCode (ac-clang--get-source-code))))


(defun ac-clang--send-inclusion-command (&optional _args)
  (ac-clang--with-widening
    (ac-clang--send-command :CommandType "Session"
                            :CommandName "INCLUSION"
                            :SessionName ac-clang--session-name
                            :Line (line-number-at-pos (point))
                            :Column (ac-clang--column-number-at-pos (point))
                            :SourceCode (ac-clang--get-source-code))))


(defun ac-clang--send-definition-command (&optional _args)
  (ac-clang--with-widening
    (ac-clang--send-command :CommandType "Session"
                            :CommandName "DEFINITION"
                            :SessionName ac-clang--session-name
                            :Line (line-number-at-pos (point))
                            :Column (ac-clang--column-number-at-pos (point))
                            :SourceCode (ac-clang--get-source-code))))


(defun ac-clang--send-declaration-command (&optional _args)
  (ac-clang--with-widening
    (ac-clang--send-command :CommandType "Session"
                            :CommandName "DECLARATION"
                            :SessionName ac-clang--session-name
                            :Line (line-number-at-pos (point))
                            :Column (ac-clang--column-number-at-pos (point))
                            :SourceCode (ac-clang--get-source-code))))


(defun ac-clang--send-smart-jump-command (&optional _args)
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

(defun ac-clang--process-filter (process output)
  ;; IPC packet receive phase.
  (setq ac-clang--status 'receive)

  (let ((receive-buffer (process-buffer process))
        (receive-buffer-marker (process-mark process))
        profile-plist)

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
      (when (> (ac-clang--count-transaction) 0)
        ;; IPC packet decode phase.
        (setq ac-clang--status 'transaction)
        (ac-clang--mark-profiler profile-plist :packet-receive)
        (setq ac-clang--command-result-data (ac-clang--decode-received-packet receive-buffer))
        (ac-clang--mark-profiler profile-plist :packet-decode)

        (let* ((transaction-id (plist-get ac-clang--command-result-data :RequestId))
               (command-error (plist-get ac-clang--command-result-data :Error))
               (profiles (plist-get ac-clang--command-result-data :Profiles))
               (transaction (ac-clang--unregister-transaction transaction-id)))

          (when command-error
            ;; server side error.
            (message "ac-clang : server command error! : %s" command-error))

          (unless transaction
            (message "ac-clang : transaction not found! : %S" transaction-id))

          (when (and transaction (not command-error))
            ;; client side execution phase.

            ;; setup transaction
            (let (;;(transaction-buffer (plist-get transaction :buffer))
                  (transaction-receiver (plist-get transaction :receiver))
                  (transaction-args (plist-get transaction :args)))

              ;; execute transaction receiver.
              (unless (ignore-errors
                        (funcall transaction-receiver ac-clang--command-result-data transaction-args)
                        t)
                (message "ac-clang : transaction(%d) receiver function execute error! : %s" transaction-id transaction)))
            ;; clear current result data.
            ;; (setq ac-clang--command-result-data nil)
            (ac-clang--mark-profiler profile-plist :transaction-receiver)
            (ac-clang--append-profiler transaction-id profile-plist)
            (ac-clang--display-profiler transaction-id)
            (when profiles
              (ac-clang--display-server-profiler profiles))
            )))

      ;; clear receive-buffer for next packet.
      (with-current-buffer receive-buffer
        (erase-buffer))
      (setq ac-clang--status 'idle))))


(defun ac-clang--decode-received-packet (buffer)
  "Result value is property-list(s-expression) that converted from packet."
  (with-current-buffer buffer
    ;; (1- (point-max)) is exclude packet termination character.
    (funcall ac-clang--packet-decoder (buffer-substring-no-properties (point-min) (1- (point-max))))))




;;;
;;; sender function for IPC
;;;

(defun ac-clang-get-server-specification ()
  (interactive)

  (when ac-clang--server-process
    (ac-clang--request-transaction #'ac-clang--send-server-specification-command #'ac-clang--receive-server-specification nil)))


(defun ac-clang--receive-server-specification (data _args)
  (let ((results (plist-get data :Results)))
    (message "ac-clang : server-specification %S" results)))




;;;
;;; The session control functions
;;;

(defun ac-clang-reparse-buffer ()
  (when ac-clang--server-process
    (ac-clang--send-reparse-command)))


(defun ac-clang-update-cflags ()
  (interactive)

  (when ac-clang--activate-p
    ;; (message "ac-clang-update-cflags %s" ac-clang--session-name)
    (ac-clang--send-cflags-command)))


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
            (apply #'start-process
                   ac-clang--process-name ac-clang--process-buffer-name
                   ac-clang--server-executable (ac-clang--build-server-launch-options))))

    (if ac-clang--server-process
        (progn
          ;; transaction initialize
          (setq ac-clang--status 'idle)
          (ac-clang--clear-transaction)

          ;; packet encoder/decoder configuration
          (setq ac-clang--packet-encoder (plist-get (plist-get ac-clang--packet-encoder/decoder-infos ac-clang-server-input-data-type) :encoder))
          (setq ac-clang--packet-decoder (plist-get (plist-get ac-clang--packet-encoder/decoder-infos ac-clang-server-output-data-type) :decoder))

          ;; process configuration
          (set-process-coding-system ac-clang--server-process
                                     (coding-system-change-eol-conversion buffer-file-coding-system nil)
                                     'binary)
          (set-process-filter ac-clang--server-process #'ac-clang--process-filter)
          (set-process-query-on-exit-flag ac-clang--server-process nil)

          ;; server configuration
          (ac-clang--send-clang-parameters-command)
          t)
      (display-warning 'ac-clang "clang-server launch failed.")
      nil)))


(defun ac-clang-shutdown-server ()
  (interactive)

  (when ac-clang--server-process
    (ac-clang--send-shutdown-command)

    (setq ac-clang--status 'shutdown)

    (setq ac-clang--server-process nil)
    t))


(defun ac-clang-update-clang-parameters ()
  (interactive)

  (when ac-clang--server-process
    (ac-clang--send-clang-parameters-command)
    t))


(defun ac-clang-reset-server ()
  (interactive)

  (when ac-clang--server-process
    (cl-dolist (buffer ac-clang--activate-buffers)
      (with-current-buffer buffer 
        (ac-clang-deactivate)))
    (ac-clang--send-reset-server-command)))


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




(defun ac-clang--check-server-require-version-p ()
  (let ((result (shell-command-to-string (format "%s --version" ac-clang--server-executable))))
    (when (string-match "server version \\([0-9]+\\)\\.\\([0-9]+\\)\\.\\([0-9]+\\)" result)
      (let ((major (string-to-number (match-string 1 result)))
            (minor (string-to-number (match-string 2 result)))
            (maintenance (string-to-number (match-string 3 result)))
            (rq-major (nth 0 ac-clang--server-require-version))
            (rq-minor (nth 1 ac-clang--server-require-version))
            (rq-maintenance (nth 2 ac-clang--server-require-version)))
        (or (> major rq-major) (and (= major rq-major) (or (> minor rq-minor) (and (= minor rq-minor) (>= maintenance rq-maintenance)))))))))




(defun ac-clang-initialize ()
  (interactive)

  ;; server binary decide
  (unless ac-clang--server-executable
    (setq ac-clang--server-executable (executable-find (or (plist-get ac-clang--server-binaries ac-clang-server-type) ""))))

  (when ac-clang--server-executable
    (unless (ac-clang--check-server-require-version-p)
      (setq ac-clang--server-executable nil)
      (display-warning 'ac-clang (format "clang-server binary is old. please replace new binary. require version is %S over." ac-clang--server-require-version))))

  ;; (message "ac-clang-initialize")
  (if ac-clang--server-executable
      (when (ac-clang-launch-server)
        ;; Change popup package used for auto-complete
        (setq ac-quick-help-prefer-pos-tip ac-clang-quick-help-prefer-pos-tip-p)

        ;; Optional keybindings
        (define-key ac-mode-map (kbd "M-.") #'ac-clang-jump-smart)
        (define-key ac-mode-map (kbd "M-,") #'ac-clang-jump-back)
        ;; (define-key ac-mode-map (kbd "C-c `") #'ac-clang-diagnostics)) 

        (defadvice flymake-on-timer-event (around ac-clang--flymake-suspend-advice last activate)
          (unless ac-clang--snippet-expanding-p
            ad-do-it))

        (when (and (eq system-type 'windows-nt) (boundp 'w32-pipe-read-delay) (> w32-pipe-read-delay 0))
          (display-warning 'ac-clang "Please set the appropriate value for `w32-pipe-read-delay'. Because a pipe delay value is large value. Ideal value is 0. see help of `w32-pipe-read-delay'."))

        (add-hook 'kill-emacs-hook #'ac-clang-finalize)

        t)
    (display-warning 'ac-clang "clang-server binary not found.")
    nil))


(defun ac-clang-finalize ()
  (interactive)

  ;; (message "ac-clang-finalize")
  (when (ac-clang-shutdown-server)
    (define-key ac-mode-map (kbd "M-.") nil)
    (define-key ac-mode-map (kbd "M-,") nil)
    ;; (define-key ac-mode-map (kbd "C-c `") nil)

    (ad-disable-advice 'flymake-on-timer-event 'around 'ac-clang--flymake-suspend-advice)

    (setq ac-clang--server-executable nil)

    (when ac-clang-tmp-pch-automatic-cleanup-p
      (ac-clang--clean-tmp-pch))

    t))





(provide 'clang-server)

;; Local Variables:
;; coding: utf-8
;; indent-tabs-mode: nil
;; End:

;;; clang-server.el ends here
