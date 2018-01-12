;;; ac-clang-cc.el --- Auto Completion source by libclang for GNU Emacs -*- lexical-binding: t; -*-

;;; last updated : 2018/01/12.15:52:45

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
(require 'ac-clang)



;; cc == compile-command
;; cdb == compilation database


(defconst ac-clang-cdb--project-buffer-name-fmt "*CDB Project<%s>*")


(defvar ac-clang-cdb--db nil
  "Database for Compilation Database(compile_commands.json)")

(defvar ac-clang-cdb--active-projects nil
  "compile_commands.json database")


;; the project name(per cc-file buffer)
(defvar-local ac-clang-cdb--db-name nil)

;; source code buffer belonging to the project name(per source code buffer)
(defvar-local ac-clang-cdb--source-code-belonging-db-name nil)



;; auto-complete ac-sources backup
(defvar-local ac-clang-cdb--ac-sources-backup nil)

;; ac-clang cflags backup
(defvar-local ac-clang-cdb--cflags-backup nil)



;; project buffer display update var.
;; usage: the control usually use let bind.
(defvar ac-clang-cdb-display-update-p t)


;; diplay allow & order
(defvar ac-clang-cdb-display-allow-properties '(:project-buffer
                                                :project-file
                                                :allow-cedet-p
                                                :allow-ac-clang-p
                                                :allow-flymake-p
                                                :cedet-root-path
                                                :cedet-spp-table
                                                :flymake-back-end
                                                :flymake-manually-p
                                                :flymake-manually-back-end
                                                :target-buffers))
                                        
;; diplay properties
(defvar ac-clang-cdb--display-property-types '(:project-buffer value
                                               :project-file path
                                               :allow-cedet-p value
                                               :allow-ac-clang-p value
                                               :allow-flymake-p value
                                               :cedet-root-path path
                                               :cedet-spp-table value
                                               :flymake-back-end value
                                               :flymake-manually-p value
                                               :flymake-manually-back-end value
                                               :target-buffers buffer))



(defvar ac-clang-cdb-flymake-error-display-style 'popup
  "flymake error message display style symbols
`popup'        : popup display
`mini-buffer'  : mini-buffer display
`nil'          : user default style")


(defvar-local ac-clang-cdb--flymake-back-end 'native
  "flymake back-end symbols
`native'       : native
`clang-server' : clang-server
`nil'          : native back-end")

(defvar-local ac-clang-cdb--flymake-manually-back-end nil
  "flymake manually mode back-end symbols
`native'       : native
`clang-server' : clang-server
`nil'          : inherit ac-clang-cdb--flymake-back-end value")


;; (defconst ac-clang-cdb--flymake-allowed-file-name-masks '(("\\.\\(?:[ch]\\(?:pp\\|xx\\|\\+\\+\\)?\\|CC\\)\\'" ac-clang-cdb--flymake-command-generator)))
(defconst ac-clang-cdb--flymake-allowed-file-name-masks '(("\\.\\(?:[ch]\\(?:pp\\|xx\\|\\+\\+\\)?\\|CC\\)\\'" flymake-simple-make-init)))

(defconst ac-clang-cdb--flymake-err-line-patterns
  '(
    ;; gcc
    native
    (("^\\(\\(?:[a-zA-Z]:\\)?[^:(\t\n]+\\):\\([0-9]+\\):\\([0-9]+\\)[ \t\n]*:[ \t\n]*\\(\\(?:error\\|warning\\|fatal error\\):\\(?:.*\\)\\)" 1 2 3 4))

    ;; clang 3.3.0 - 5.0.0
    clang-server
    (("^\\(\\(?:[a-zA-Z]:\\)?[^:(\t\n]+\\):\\([0-9]+\\):\\([0-9]+\\)[ \t\n]*:[ \t\n]*\\(\\(?:error\\|warning\\|fatal error\\):\\(?:.*\\)\\)" 1 2 3 4)))

  "  (REGEXP FILE-IDX LINE-IDX COL-IDX ERR-TEXT-IDX).")



;; for Project Buffer keymap
(defvar ac-clang-cdb--mode-filter-map
  (let ((map (make-sparse-keymap)))
    (define-key map (kbd "RET") #'ac-clang-cdb--keyboard-visit-target)
    (define-key map (kbd "C-z") #'ac-clang-cdb--keyboard-visit-target-other-window)
    ;; (define-key map [(mouse-1)] #'ibuffer-mouse-toggle-mark)
    (define-key map [(mouse-1)] #'ac-clang-cdb--mouse-visit-target)
    ;; (define-key map [down-mouse-3] #'ibuffer-mouse-popup-menu)
    map))



(defun ac-clang-cdb--register-db (cc-file)
  (ac-clang-cdb--unregister-db cc-file)
  (add-to-list 'ac-clang-cdb--db (ac-clang-cdb--parse-cc-file cc-file)))

(defun ac-clang-cdb--unregister-db (cc-file)
  (setq ac-clang-cdb--db (delete (assoc-string cc-file ac-clang-cdb--db) ac-clang-cdb--db)))

(defun ac-clang-cdb--query-cdb (cc-file)
  "return detail of cdb"
  (cdr (assoc-string cc-file ac-clang-cdb--db)))

(cl-defun ac-clang-cdb--query-cc (cc-file &optional (file-name buffer-file-name))
  "return compile command"
  (let ((cdb (ac-clang-cdb--query-cdb cc-file)))
    (cl-dolist (cc cdb)
      (when (string= file-name (plist-get cc :file))
        (cl-return-from ac-clang-cdb--query-cc cc)))))
    



(defun ac-clang-cdb--register-project (db-name details)
  (ac-clang-cdb--unregister-project db-name)
  (add-to-list 'ac-clang-cdb--active-projects `(,db-name . ,details)))

(defun ac-clang-cdb--unregister-project (db-name)
  (setq ac-clang-cdb--active-projects (delete (assoc-string db-name ac-clang-cdb--active-projects) ac-clang-cdb--active-projects)))

(defun ac-clang-cdb--query-project (db-name)
  "return detail of cdb"
  (cdr (assoc-string db-name ac-clang-cdb--active-projects)))

(defun ac-clang-cdb--query-current-project ()
  "return detail of cdb"
  (ac-clang-cdb--query-project (or ac-clang-cdb--db-name ac-clang-cdb--source-code-belonging-db-name)))


(defun ac-clang-cdb--parse-cc-file (cc-file)
  (when (and cc-file (file-exists-p cc-file))
    (let* ((json-object-type 'plist)
           (json-array-type 'vector)
           (cc-objects (json-read-file cc-file))
           cc-details)

      (setq cc-details (mapcar (lambda (cc-object)
                                 (ac-clang-cdb--parse-cc-object cc-object))
                               cc-objects))
      (append `(,cc-file) cc-details))))


(defun ac-clang-cdb--parse-cc-object (cc-object)
  "This parser is compatible at compile_commands.json of cmake format and Bear format.
return object is parsed cc-object"
  (let* ((directory (plist-get cc-object :directory))
         (file (plist-get cc-object :file))
         (arguments (or (plist-get cc-object :arguments) (plist-get cc-object :command)))
         canonicalized-arguments
         cflags)
    (when (not (file-name-absolute-p file))
      (setq file (expand-file-name file directory)))
    (if (vectorp arguments)
        (setq canonicalized-arguments (append arguments nil))
      (setq arguments (split-string arguments))
      (cl-dolist (element arguments)
        (if (string-prefix-p "-" element)
            (push element canonicalized-arguments)
          (when canonicalized-arguments
            (setf (car canonicalized-arguments) (concat (car canonicalized-arguments) " " element)))))
      (setq canonicalized-arguments (nreverse canonicalized-arguments)))
    ;; exclude non switch arguments
    (mapc (lambda (element)
            (when (string-prefix-p "-" element)
              (push element cflags)))
          canonicalized-arguments)
    (setq cflags (nreverse cflags))
    `(:file ,file :cflags ,cflags)))



(cl-defun ac-clang-cdb--search-cc-file (&optional (current-path buffer-file-name))
  (let ((cc-file-name "compile_commands.json")
        cc-file-path
        prev-search-path)
    ;; directory
    (unless (file-directory-p current-path)
      (setq current-path (file-name-directory current-path)))
    ;; search current to parent
    (while (not (string= current-path prev-search-path))
      (setq cc-file-path (expand-file-name cc-file-name current-path))
      (if (file-exists-p cc-file-path)
          (cl-return-from ac-clang-cdb--search-cc-file cc-file-path)
        (setq prev-search-path current-path)
        (setq current-path (file-name-directory (directory-file-name current-path)))))))


(defun ac-clang-cdb--target-buffer-p (db-name)
  ;; major-mode check
  (when (and (memq major-mode '(c++-mode c-mode)) buffer-file-name)
    (ac-clang-cdb--query-cc db-name)))


;; すでにオープンされているバッファでプロジェクトに所属しているものを集める
(defun ac-clang-cdb--collect-target-buffer (db-name)
  (let* ((buffers (buffer-list))
         target-buffers)

    (cl-dolist (buffer buffers)
      (with-current-buffer buffer
        ;; file belonging check
        (when (ac-clang-cdb--target-buffer-p db-name)
          (push buffer target-buffers))))
    target-buffers))



(cl-defun ac-clang-cdb--evaluate-buffer-by-active-projects ()
  (interactive)

  (message "ac-clang-cdb--evaluate-buffer-by-active-projects : belong : %S" ac-clang-cdb--source-code-belonging-db-name)
  (backtrace)

  ;; This buffer already active
  (when ac-clang-cdb--source-code-belonging-db-name
    (cl-return-from ac-clang-cdb--evaluate-buffer-by-active-projects t))

  ;; Search active projects
  (cl-dolist (project ac-clang-cdb--active-projects)
    (let* ((db-name (car project)))
      (message "ac-clang-cdb--evaluate-buffer-by-active-projects : db-name : %S" db-name)
      (when (ac-clang-cdb--target-buffer-p db-name)
        (message "ac-clang-cdb--evaluate-buffer-by-active-projects : ac-clang-cdb--target-buffer-p : pass")
        (ac-clang-cdb--attach-to-project db-name)
        (cl-return-from ac-clang-cdb--evaluate-buffer-by-active-projects t)))))


(cl-defun ac-clang-cdb--evaluate-buffer-by-cdb ()
  (interactive)

  (message "ac-clang-cdb--evaluate-buffer-by-cdb call : belong : %S" ac-clang-cdb--source-code-belonging-db-name)
  (backtrace)

  ;; This buffer already active
  (when ac-clang-cdb--source-code-belonging-db-name
    (cl-return-from ac-clang-cdb--evaluate-buffer-by-cdb t))

  ;; Search cc-file from the current path to the parent path.
  (let ((cc-file (ac-clang-cdb--search-cc-file buffer-file-name)))
    (when (and cc-file (not (ac-clang-cdb--query-cdb cc-file)))
      (ac-clang-cdb--register-db cc-file)))
  
  ;; Search from DB
  (cl-dolist (db-element ac-clang-cdb--db)
    (message "ac-clang-cdb--evaluate-buffer-by-cdb db-element    : %S" db-element)
    (let ((db-name (car db-element))
          ;; (cdb (cdr db-element))
          )
      (when (ac-clang-cdb--target-buffer-p db-name)
        (message "ac-clang-cdb--evaluate-buffer-by-cdb : pass target-buffer-p-1")
        (if (ac-clang-cdb--query-project db-name)
            ;; already active
            (progn
              (message "ac-clang-cdb--evaluate-buffer-by-cdb : pass ac-clang-cdb--attach-to-project")
              (ac-clang-cdb--attach-to-project db-name)
              (cl-return-from ac-clang-cdb--evaluate-buffer-by-cdb t))
          ;; not active
          (if (ac-clang-cdb-activate-project db-name)
              (progn
                (message "ac-clang-cdb--evaluate-buffer-by-cdb : pass ac-clang-cdb--active-projects")
                (cl-return-from ac-clang-cdb--evaluate-buffer-by-cdb t))
            (message "ac-clang-cdb--evaluate-buffer-by-cdb : fail ac-clang-cdb--active-projects")
            (cl-return-from ac-clang-cdb--evaluate-buffer-by-cdb nil)))))))


(defun ac-clang-cdb--evaluate-buffer (auto-active-p)
  (if auto-active-p
      (ac-clang-cdb--evaluate-buffer-by-cdb)
    (ac-clang-cdb--evaluate-buffer-by-active-projects)))


(defun ac-clang-cdb--create-cflags (db-name &optional additional-options)
  (let* ((default-options '(
                            ;; -cc1 options
                            ;; libclang3.1 は↓の渡し方しないとだめ(3.2/3.3は未調査)
                            ;; -no*inc系オプションを個別に渡すと include サーチの動作がおかしくなる
                            ;; -isystem で正常なパスを渡していても Windows.h は見に行けるが、 stdio.h vector などを見に行っていないなど

                            ;; なんか -x c++ だと -nostdinc だめ？とりあえず外しておく(Clang3.3)
                            ;; "-nobuiltininc -nostdinc -nostdinc++ -nostdsysteminc"
                            ;; "-nobuiltininc -nostdinc++ -nostdsysteminc"
                            ;; "-nobuiltininc" "-nostdinc++" "-nostdsysteminc"

                            ;; "-code-completion-macros" "-code-completion-patterns"
                            ;; "-code-completion-brief-comments"

                            "-Wno-unused-value" "-Wno-#warnings" "-Wno-microsoft" "-Wc++11-extensions"
                            ;; undef all system defines
                            ))
         (db-clang-cflags (plist-get (ac-clang-cdb--query-cc db-name) :cflags))
         (clang-cflags (append default-options db-clang-cflags additional-options)))

    clang-cflags))



(defun ac-clang-cdb--switch-to-buffer-other-frame (buffer)
  (let ((current-frame (selected-frame)))
    (switch-to-buffer-other-frame buffer)
    (raise-frame current-frame)))

(defun ac-clang-cdb--split-window (buffer)
  (unless (get-buffer-window-list buffer)
    (let ((target-window (if (one-window-p) (split-window-below) (next-window))))
      (set-window-buffer target-window buffer))))

(defun ac-clang-cdb--visit-buffer (point switch-function)
  (let* ((target-buffer (get-text-property point 'value)))
    (if target-buffer
        (apply switch-function target-buffer nil)
      (error "buffer no present"))))

(defun ac-clang-cdb--visit-path (point switch-function)
  (let* ((target-path (get-text-property point 'value)))
    (if target-path
        (apply switch-function target-path nil)
      (error "path no present"))))


(defun ac-clang-cdb--keyboard-visit-target ()
  "Toggle the display status of the filter group on this line."
  (interactive)

  (cl-case (get-text-property (point) 'target)
    (buffer
     (ac-clang-cdb--visit-buffer (point) #'switch-to-buffer))
    (path
     (ac-clang-cdb--visit-path (point) #'find-file))))


(defun ac-clang-cdb--keyboard-visit-target-other-window ()
  "Toggle the display status of the filter group on this line."
  (interactive)

  (cl-case (get-text-property (point) 'target)
    (buffer
     (ac-clang-cdb--visit-buffer (point) #'ac-clang-cdb--split-window))
    (path
     (ac-clang-cdb--visit-path (point) #'find-file-other-window))))


(defun ac-clang-cdb--mouse-visit-target (_event)
  "Toggle the display status of the filter group chosen with the mouse."
  (interactive "e")

  (cl-case (get-text-property (point) 'target)
    (buffer
     (ac-clang-cdb--visit-buffer (point) #'switch-to-buffer))
    (path
     (ac-clang-cdb--visit-path (point) #'find-file))))


;; プロジェクトディテールをプロジェクトバッファに表示する
(defun ac-clang-cdb--display-project-details (db-name)
  (when ac-clang-cdb-display-update-p
    (let* ((details (ac-clang-cdb--query-project db-name))
           (project-buffer (plist-get details :project-buffer)))
      (when project-buffer
        (with-current-buffer project-buffer
          (let ((buffer-read-only nil))
            (erase-buffer)
            (goto-char (point-min))

            (cl-dolist (property ac-clang-cdb-display-allow-properties)
              (let ((type (plist-get ac-clang-cdb--display-property-types property))
                    (value (plist-get details property))
                    (start-pos (point)))
                (cl-case type
                  (path
                   (insert
                    (format "%-30s : " property)
                    (propertize (format "%s" value) 'face 'font-lock-keyword-face)
                    "\n")
                   (add-text-properties start-pos (1- (point)) `(mouse-face highlight))
                   (add-text-properties start-pos (point) `(target ,type value ,value keymap ,ac-clang-cdb--mode-filter-map)))
                  (buffer
                   (insert (format "%-30s :\n" property))
                   (cl-dolist (buffer value)
                     (setq start-pos (point))
                     (insert
                      (format " -%-28s : " "buffer-name")
                      (propertize (format "%-30s : %s" buffer (buffer-file-name buffer)) 'face 'font-lock-keyword-face)
                      "\n")
                     (add-text-properties start-pos (1- (point)) `(mouse-face highlight))
                     (add-text-properties start-pos (point) `(target ,type value ,buffer keymap ,ac-clang-cdb--mode-filter-map))))
                  (value
                   (insert (format "%-30s : %s\n" property value))))))))))))


;; CEDET Project.ede を生成する
(defun ac-clang-cdb--create-ede-project-file (ede-proj-file db-name)
  (let* ((proj-name db-name)
         (file-name (file-name-nondirectory ede-proj-file)))
    (with-temp-file ede-proj-file
      (insert ";; Object " proj-name "\n"
              ";; EDE Project File.\n"
              "(ede-proj-project \"" proj-name "\"
                  :file \"" file-name "\"
                  :name \"" proj-name "\"
                  :targets 'nil
                  )"))))


(defadvice flymake-start-syntax-check-process (around ac-clang-cdb--flymake-start-syntax-check-process-advice (cmd args dir) activate)
  (cl-case ac-clang-cdb--flymake-back-end
    (clang-server
     (ac-clang-diagnostics))
    (t
     ad-do-it)))


;; (defun ac-clang-cdb--flymake-command-generator ()
;;   (interactive)
;;   (let* ((db-name ac-clang-cdb--source-code-belonging-db-name)
;;          (compile-file (flymake-init-create-temp-buffer-copy 'flymake-create-temp-inplace))

;;          (cedet-file-name (cedet-directory-name-to-file-name compile-file))
;;          (cedet-project-path (cedet-directory-name-to-file-name (ac-clang-cdb-flags--create-project-path db-name)))
;;          (extract-file-name (substring cedet-file-name (1- (abs (compare-strings cedet-project-path nil nil cedet-file-name nil nil)))))

;;          (details (ac-clang-cdb--query-project db-name))
;;          (db-path (plist-get details :db-path))
;;          (version (plist-get details :version))
;;          (toolset (plist-get details :toolset))
;;          (md5-name-p (plist-get details :md5-name-p))
;;          (fix-file-name (if md5-name-p (md5 extract-file-name) extract-file-name))
;;          (msb-rsp-file (expand-file-name (concat fix-file-name ".flymake.rsp.ac-clang-cdb") db-path))
;;          (log-file (expand-file-name (concat fix-file-name ".flymake.log.ac-clang-cdb") db-path)))

;;     ;; create rsp file
;;     (unless (file-exists-p msb-rsp-file)
;;       (let* ((project-file (plist-get details :project-file))
;;              (platform (plist-get details :platform))
;;              (configuration (plist-get details :configuration))

;;              (logger-encoding "UTF-8")
;;              (project-path (file-name-directory project-file))
;;              (msb-target-file (expand-file-name ac-clang-cdb--flymake-vcx-proj-name project-path))
;;              (msb-flags (list
;;                          (ac-clang-cdb-env--create-msb-flags "/p:"
;;                                                      `(("ImportProjectFile=%S"       .       ,project-file)
;;                                                        ("Platform=%S"                .       ,platform)
;;                                                        ("Configuration=%S"           .       ,configuration)
;;                                                        ("CompileFile=%S"             .       ,compile-file)
;;                                                        ;; IntDir,OutDirは末尾にスラッシュが必須(MSBuildの仕様)
;;                                                        ("IntDir=%S"                  .       ,db-path)
;;                                                        ("OutDir=%S"                  .       ,db-path)))
;;                          (ac-clang-cdb-env--create-msb-flags "/flp:"
;;                                                      `(("Verbosity=%s"               .       "normal")
;;                                                        ("LogFile=%S"                 .       ,log-file)
;;                                                        ("Encoding=%s"                .       ,logger-encoding)
;;                                                        ("%s"                         .       "NoSummary")))
;;                          "/noconsolelogger"
;;                          "/nologo")))

;;         (ac-clang-cdb-env--create-msb-rsp-file msb-rsp-file msb-target-file msb-flags)))

;;     (list 
;;      ac-clang-cdb-env--invoke-command
;;      (ac-clang-cdb-env--build-msb-command-args version toolset msb-rsp-file log-file))))


;; error message display to Minibuf
(defun ac-clang-cdb--flymake-display-current-line-error-by-minibuf ()
  "Displays the error/warning for the current line in the minibuffer"

  (let* ((line-no (line-number-at-pos))
         (line-err-info-list (nth 0 (flymake-find-err-info flymake-err-info line-no)))
         (count (length line-err-info-list)))
    (while (> count 0)
      (when line-err-info-list
        (let* ((text (flymake-ler-text (nth (1- count) line-err-info-list)))
               (line (flymake-ler-line (nth (1- count) line-err-info-list))))
          (message "[%s] %s" line text)))
      (setq count (1- count)))))

;; error message display to popup-tip
;; use popup.el (include auto-complete packages)
(defun ac-clang-cdb--flymake-display-current-line-error-by-popup ()
  "Display a menu with errors/warnings for current line if it has errors and/or warnings."

  (let* ((line-no (line-number-at-pos))
         (errors (nth 0 (flymake-find-err-info flymake-err-info line-no)))
         (texts (mapconcat (lambda (x) (flymake-ler-text x)) errors "\n")))
    (when texts
      (popup-tip texts))))

(defun ac-clang-cdb--flymake-display-current-line-error ()
  (cl-case ac-clang-cdb-flymake-error-display-style
    (popup
     (ac-clang-cdb--flymake-display-current-line-error-by-popup))
    (mini-buffer
     (ac-clang-cdb--flymake-display-current-line-error-by-minibuf))))



(defun ac-clang-cdb--setup-project-feature-ac-clang (_db-name status)
  (cl-case status
    (enable
     nil
     )
    (disable
     nil)))

(defun ac-clang-cdb--setup-buffer-feature-ac-clang (db-name status)
  (cl-case status
    (enable
     ;; backup value
     (push ac-clang-cflags ac-clang-cdb--cflags-backup)

     ;; set database value
     (setq ac-clang-cflags (ac-clang-cdb--create-cflags db-name))

     ;; buffer modified > do activation
     ;; buffer not modify > delay activation
     (ac-clang-activate-after-modify))
    (disable
     ;; always deactivation
     (ac-clang-deactivate)

     ;; restore value
     (setq ac-clang-cflags (pop ac-clang-cdb--cflags-backup)))))


;; CEDET セットアップ関数
(defun ac-clang-cdb--setup-project-feature-cedet (db-name status)
  (let* ((details (ac-clang-cdb--query-project db-name))
         (project-path (file-name-directory (plist-get details :project-file)))

         ;; cedet-root-path が未設定の場合はプロジェクトファイルのパスから生成する
         ;; この場合、ディレクトリ構成によっては正常に動作しないケースもある
         (cedet-root-path (or (plist-get details :cedet-root-path) project-path))
         (cedet-spp-table (plist-get details :cedet-spp-table))
         ;; (system-inc-paths (ac-clang-cdb--convert-to-cedet-style-path (ac-clang-cdb-flags--query-cflag db-name "CFLAG_SystemIncludePath")))
         ;; (additional-inc-paths (ac-clang-cdb--convert-to-cedet-style-path (ac-clang-cdb-flags--query-cflag db-name "CFLAG_AdditionalIncludePath") project-path))
         (system-inc-paths nil)
         (additional-inc-paths nil)
         ;; (project-header-match-regexp "\\.\\(h\\(h\\|xx\\|pp\\|\\+\\+\\)?\\|H\\|inl\\)$\\|\\<\\w+$")
         (project-header-match-regexp "\\.\\(h\\(h\\|xx\\|pp\\|\\+\\+\\)?\\|H\\|inl\\)$")
         (ede-proj-file (expand-file-name (concat db-name ".ede") cedet-root-path))
         additional-inc-rpaths)

    (cl-case status
      (enable
       ;; generate relative path(CEDET format)
       (cl-dolist (path additional-inc-paths)
         (setq path (file-relative-name path cedet-root-path))
         ;; All path is relative from cedet-root-path.
         ;; And relative path string require starts with "/". (CEDET :include-path format specification)
         (setq path (concat "/" path))
         ;; (add-to-list 'additional-inc-rpaths (file-name-as-directory path) t))
         (cl-pushnew (file-name-as-directory path) additional-inc-rpaths :test #'equal))
       (setq additional-inc-rpaths (nreverse additional-inc-rpaths))

       ;; generate Project.ede file
       ;; (print "ede-proj-file")
       ;; (print ede-proj-file)
       (unless (file-readable-p ede-proj-file)
         (ac-clang-cdb--create-ede-project-file ede-proj-file db-name))

       ;; (print "ede-cpp-root-project")
       ;; (print ede-proj-file)
       ;; (print cedet-root-path)
       ;; (print additional-inc-rpaths)
       ;; (print system-inc-paths)
       ;; (print project-header-match-regexp)
       ;; (print cedet-spp-table)

       (semantic-reset-system-include 'c-mode)
       (semantic-reset-system-include 'c++-mode)
       (setq semantic-c-dependency-system-include-path nil)
       (ede-cpp-root-project db-name ;ok
                             :file ede-proj-file ;ok
                             :directory cedet-root-path ; ok
                             :include-path additional-inc-rpaths ; :directoryからの相対パスで指定
                             :system-include-path system-inc-paths ;ok
                             :header-match-regexp project-header-match-regexp ;ok
                             :spp-table cedet-spp-table ;ok
                             :spp-files nil ;ok
                             :local-variables nil ;ok ede:project-local-variables
                             ))
      (disable
       nil))))

(defun ac-clang-cdb--setup-buffer-feature-cedet (_db-name status)
  (cl-case status
    (enable
     ;; backup value
     (push ac-sources ac-clang-cdb--ac-sources-backup)
     ;; auto-complete ac-sources setup(use semantic)
     (setq ac-sources '(
                        ;; ac-source-dictionary
                        ac-source-semantic
                        ac-source-semantic-raw
                        ac-source-imenu
                        ;; ac-source-words-in-buffer
                        ;; ac-source-words-in-same-mode-buffers
                        ))

     ;; Force a full refresh of the current buffer's tags.
     ;; (semantic-force-refresh)
     )
    (disable
     ;; restore value
     (setq ac-sources (pop ac-clang-cdb--ac-sources-backup)))))



;; flymake セットアップ関数
(defun ac-clang-cdb--setup-project-feature-flymake (_db-name status)
  (cl-case status
    (enable
     ;; (unless (rassoc '(msvc--flymake-command-generator) flymake-allowed-file-name-masks)
     ;;   (push `(,msvc--flymake-target-pattern msvc--flymake-command-generator) flymake-allowed-file-name-masks)))
     nil)
    (disable
     ;; (setq flymake-allowed-file-name-masks (delete (rassoc '(msvc--flymake-command-generator) flymake-allowed-file-name-masks) flymake-allowed-file-name-masks)))
     nil)))

(defun ac-clang-cdb--setup-buffer-feature-flymake (db-name status)
  (let* ((details (ac-clang-cdb--query-project db-name))
         (back-end (plist-get details :flymake-back-end))
         (manually-p (plist-get details :flymake-manually-p))
         (manually-back-end (plist-get details :flymake-manually-back-end)))

    (cl-case status
      (enable
       (when back-end
         (setq ac-clang-cdb--flymake-back-end back-end))
       (setq ac-clang-cdb--flymake-manually-back-end (or manually-back-end ac-clang-cdb--flymake-back-end))
       (set (make-local-variable 'flymake-allowed-file-name-masks) ac-clang-cdb--flymake-allowed-file-name-masks)
       (set (make-local-variable 'flymake-err-line-patterns) (plist-get ac-clang-cdb--flymake-err-line-patterns ac-clang-cdb--flymake-manually-back-end))
       ;; 複数バッファのflymakeが同時にenableになるとflymake-processでpipe errorになるのを抑制
       (set (make-local-variable 'flymake-start-syntax-check-on-find-file) nil)

       (flymake-mode-on)
       (when manually-p
         (defadvice flymake-on-timer-event (around ac-clang-cdb--flymake-suspend-advice last activate)
           (let* ((details (ac-clang-cdb--query-current-project))
                  (manually-p (plist-get details :flymake-manually-p)))
             ;; (when (and details (not manually-p))
             (unless manually-p
               ad-do-it)))))
      ;; (let ((flymake-start-syntax-check-on-find-file nil))
      ;;   (flymake-mode-on)))
      (disable
       (when manually-p
         (flymake-delete-own-overlays)
         (ad-disable-advice 'flymake-on-timer-event 'around 'ac-clang-cdb--flymake-suspend-advice))
       (flymake-mode-off)

       (setq ac-clang-cdb--flymake-back-end nil)
       (setq ac-clang-cdb--flymake-manually-back-end nil)
       (set (make-local-variable 'flymake-allowed-file-name-masks) (default-value 'flymake-allowed-file-name-masks))
       (set (make-local-variable 'flymake-err-line-patterns) (default-value 'flymake-err-line-patterns))
       (set (make-local-variable 'flymake-start-syntax-check-on-find-file) (default-value 'flymake-start-syntax-check-on-find-file))))))


(defun ac-clang-cdb--attach-to-project (db-name)
  (interactive)

  (message "ac-clang-cdb--attach-to-project : db-name : %S" db-name)
  (message "ac-clang-cdb--attach-to-project : belong : %S" ac-clang-cdb--source-code-belonging-db-name)
  (let* ((details (ac-clang-cdb--query-project db-name))
         (allow-cedet-p (plist-get details :allow-cedet-p))
         (allow-ac-clang-p (plist-get details :allow-ac-clang-p))
         (allow-flymake-p (plist-get details :allow-flymake-p))
         (target-buffers (plist-get details :target-buffers)))
    ;; (print db-name)
    ;; (print details)
    ;; (print target-buffers)

    (unless ac-clang-cdb--source-code-belonging-db-name
      ;; cc-file set to local-var for project target buffer.
      (setq ac-clang-cdb--source-code-belonging-db-name db-name)

      ;; attach to project
      (push (current-buffer) target-buffers)
      (setq details (plist-put details :target-buffers target-buffers))
      ;; (print target-buffers)

      ;; (add-hook 'kill-buffer-hook #'ac-clang-cdb--detach-from-project nil t)
      ;; (add-hook 'before-revert-hook #'ac-clang-cdb--detach-from-project nil t)
      (add-hook 'kill-buffer-hook #'ac-clang-cdb-mode-off nil t)
      (add-hook 'before-revert-hook #'ac-clang-cdb-mode-off nil t)

      ;; launch allow features(launch order low > high)

      ;; ---- CEDET ----
      (when allow-cedet-p
        (ac-clang-cdb--setup-buffer-feature-cedet db-name 'enable))
      ;; ---- ac-clang ----
      (when allow-ac-clang-p
        (ac-clang-cdb--setup-buffer-feature-ac-clang db-name 'enable))
      ;; ---- flymake ----
      (when allow-flymake-p
        (ac-clang-cdb--setup-buffer-feature-flymake db-name 'enable))

      ;; プロジェクト状態をバッファへ表示
      (ac-clang-cdb--display-project-details db-name)

      t)))


(defun ac-clang-cdb--detach-from-project ()
  (interactive)

  (message "ac-clang-cdb--detach-from-project : belong : %S" ac-clang-cdb--source-code-belonging-db-name)
  (backtrace)
  (when ac-clang-cdb--source-code-belonging-db-name
    (let* ((db-name ac-clang-cdb--source-code-belonging-db-name)
           (details (ac-clang-cdb--query-project db-name))
           (allow-cedet-p (plist-get details :allow-cedet-p))
           (allow-ac-clang-p (plist-get details :allow-ac-clang-p))
           (allow-flymake-p (plist-get details :allow-flymake-p))
           (target-buffers (plist-get details :target-buffers)))

      ;; clear beglonging db-name
      (setq ac-clang-cdb--source-code-belonging-db-name nil)

      ;; detach from project
      (setq target-buffers (delete (current-buffer) target-buffers))
      (setq details (plist-put details :target-buffers target-buffers))

      (remove-hook 'kill-buffer-hook #'ac-clang-cdb-mode-off t)
      (remove-hook 'before-revert-hook #'ac-clang-cdb-mode-off t)

      ;; shutdown allow features(order hight > low)

      ;; ---- flymake ----
      (when allow-flymake-p
        (ac-clang-cdb--setup-buffer-feature-flymake db-name 'disable))
      ;; ---- ac-clang ----
      (when allow-ac-clang-p
        (ac-clang-cdb--setup-buffer-feature-ac-clang db-name 'disable))
      ;; ---- CEDET ----
      (when allow-cedet-p
        (ac-clang-cdb--setup-buffer-feature-cedet db-name 'disable))

      ;; プロジェクト状態をバッファへ表示
      (ac-clang-cdb--display-project-details db-name)

      t)))



(defvar ac-clang-cdb--project-property-symbols '(:allow-cedet-p
                                                 :allow-ac-clang-p
                                                 :allow-flymake-p
                                                 :cedet-root-path
                                                 :cedet-spp-table
                                                 :flymake-back-end
                                                 :flymake-manually-p
                                                 :flymake-manually-back-end))


(defvar ac-clang-cdb--project-property-default-value '(:allow-cedet-p t
                                                       :allow-ac-clang-p t
                                                       :allow-flymake-p t
                                                       :cedet-root-path nil
                                                       :cedet-spp-table nil
                                                       :flymake-back-end native
                                                       :flymake-manually-p nil
                                                       :flymake-manually-back-end nil))


(defun ac-clang-cdb--merge-default-property (args)
  (cl-dolist (symbol ac-clang-cdb--project-property-symbols)
    (let ((value (or (plist-get args symbol) (plist-get ac-clang-cdb--project-property-default-value symbol))))
      (setq args (plist-put args symbol value))))
  args)



(cl-defun ac-clang-cdb-activate-project (db-name &rest args)
  "attributes
-optionals
:allow-cedet-p
:allow-ac-clang-p
:allow-flymake-p
:cedet-root-path
:cedet-spp-table
:flymake-back-end
:flymake-manually-p
:flymake-manually-back-end
"
  (interactive)

  ;; (message (format "allow-ac-clang-p = %s, allow-cedet-p = %s, allow-flymake-p = %s\n" allow-ac-clang-p allow-cedet-p allow-flymake-p))
  ;; (backtrace)

  (unless db-name
    (message "ac-clang-cdb : db-name is nil.")
    (cl-return-from ac-clang-cdb-activate-project nil))

  (when (ac-clang-cdb--query-project db-name)
    (message "ac-clang-cdb : db-name is already active.")
    (cl-return-from ac-clang-cdb-activate-project nil))
  
  ;; (ac-clang-cdb--register-db db-name)

  (setq args (ac-clang-cdb--merge-default-property args))

  ;; DBリストからプロジェクトマネージャーを生成
  (let* (;; project basic information(from property)
         (project-buffer (format ac-clang-cdb--project-buffer-name-fmt db-name))
         (project-file db-name)

         ;; project basic information(from args)

         ;; project allow feature(from args)
         (allow-cedet-p (plist-get args :allow-cedet-p))
         (allow-ac-clang-p (plist-get args :allow-ac-clang-p))
         (allow-flymake-p (plist-get args :allow-flymake-p))
         (cedet-root-path (plist-get args :cedet-root-path))
         (cedet-spp-table (plist-get args :cedet-spp-table))
         (flymake-back-end (plist-get args :flymake-back-end))
         (flymake-manually-p (plist-get args :flymake-manually-p))
         (flymake-manually-back-end (plist-get args :flymake-manually-back-end))

         (target-buffers nil)
         ;; details
         )

    ;; compile_commands exist check
    (unless (ac-clang-cdb--query-cdb db-name)
      (message "ac-clang-cdb : db-name not found in database. : %s" db-name)
      (cl-return-from ac-clang-cdb-activate-project nil))

    ;; 既存バッファは削除（削除によって既存プロジェクトの削除も動作するはず）
    (when (get-buffer project-buffer)
      (kill-buffer project-buffer))

    (get-buffer-create project-buffer)

    ;; dbへ登録のみ
    ;; value が最初はnilだとわかっていても変数を入れておかないと評価時におかしくなる・・
    ;; args をそのまま渡したいが、 意図しないpropertyが紛れ込みそうなのでちゃんと指定する
    (ac-clang-cdb--register-project db-name `(
                                              :project-buffer ,project-buffer
                                              :project-file ,project-file
                                              :allow-cedet-p ,allow-cedet-p
                                              :allow-ac-clang-p ,allow-ac-clang-p
                                              :allow-flymake-p ,allow-flymake-p
                                              :cedet-root-path ,cedet-root-path
                                              :cedet-spp-table ,cedet-spp-table
                                              :flymake-back-end ,flymake-back-end
                                              :flymake-manually-p ,flymake-manually-p
                                              :flymake-manually-back-end ,flymake-manually-back-end
                                              :target-buffers ,target-buffers
                                              ))

    ;; setup project buffer
    (with-current-buffer project-buffer
      ;; db-name set local-var for ac-clang-cdb buffer
      (setq ac-clang-cdb--db-name db-name)

      ;; (add-to-list 'ac-clang-cdb--active-projects project-buffer)
      ;; 該当バッファが消されたらマネージャーから外す
      (add-hook 'kill-buffer-hook `(lambda () (ac-clang-cdb--deactivate-project ,db-name)) nil t)

      ;; launch features (per project)

      ;; ---- CEDET ----
      (when allow-cedet-p
        (ac-clang-cdb--setup-project-feature-cedet db-name 'enable))
      ;; ---- ac-clang ----
      (when allow-ac-clang-p
        (ac-clang-cdb--setup-project-feature-ac-clang db-name 'enable))
      ;; ---- flymake ----
      (when allow-flymake-p
        (ac-clang-cdb--setup-project-feature-flymake db-name 'enable))

      ;; 編集させない
      (setq buffer-read-only t))

    ;; 以下プロジェクトのセットアップが終わってから行う(CEDETなどのプロジェクト付機能のセットアップも終わっていないとだめ)
    ;; オープン済みで所属バッファを収集
    (setq target-buffers (ac-clang-cdb--collect-target-buffer db-name))

    ;; (message "target-buffers %S" target-buffers)

    ;; target buffer all attach
    (let ((ac-clang-cdb-display-update-p nil))
      (cl-dolist (buffer target-buffers)
        (with-current-buffer buffer
          (ac-clang-cdb-mode-on))))
    
    ;; プロジェクト状態をバッファへ表示
    ;; (ac-clang-cdb--display-project-details db-name)

    t))


;; プロジェクトのデアクティベーション
(defun ac-clang-cdb--deactivate-project (db-name)
  (interactive)

  (message "ac-clang-cdb--deactivate-project : db-name : %S" db-name)

  (let ((details (ac-clang-cdb--query-project db-name)))
    (when details
      (let* ((project-buffer (format ac-clang-cdb--project-buffer-name-fmt db-name))
             (allow-cedet-p (plist-get details :allow-cedet-p))
             (allow-ac-clang-p (plist-get details :allow-ac-clang-p))
             (allow-flymake-p (plist-get details :allow-flymake-p))
             (target-buffers (plist-get details :target-buffers)))

        ;; target buffers all detach
        (let ((ac-clang-cdb-display-update-p nil))
          (cl-dolist (buffer target-buffers)
            (with-current-buffer buffer
              (ac-clang-cdb-mode-off))))


        ;; shutdown features (per project)
        ;; allowed features are necessary to shutdown.
        (with-current-buffer project-buffer
          ;; ---- flymake ----
          (when allow-flymake-p
            (ac-clang-cdb--setup-project-feature-flymake db-name 'disable))
          ;; ---- ac-clang ----
          (when allow-ac-clang-p
            (ac-clang-cdb--setup-project-feature-ac-clang db-name 'disable))
          ;; ---- CEDET ----
          (when allow-cedet-p
            (ac-clang-cdb--setup-project-feature-cedet db-name 'disable))))


      ;; a project is removed from database.
      ;; (print (format "ac-clang-cdb--deactivate-project %s" db-name))
      (ac-clang-cdb--unregister-project db-name)
      t)))



(defun ac-clang-cdb-mode-feature-flymake-goto-prev-error ()
  (interactive)

  (flymake-goto-prev-error)
  (ac-clang-cdb--flymake-display-current-line-error))

(defun ac-clang-cdb-mode-feature-flymake-goto-next-error ()
  (interactive)

  (flymake-goto-next-error)
  (ac-clang-cdb--flymake-display-current-line-error))


(defun ac-clang-cdb-mode-feature-manually-flymake ()
  (interactive)
  (cl-case ac-clang-cdb--flymake-manually-back-end
    (clang-server
     ;; back end : clang-server
     (ac-clang-diagnostics))
    (t
     ;; back end : native
     (flymake-start-syntax-check))))


;; mode definitions
(defvar-local ac-clang-cdb--mode-line nil)


(defvar ac-clang-cdb--mode-key-map 
  (let ((map (make-sparse-keymap)))
    ;; (define-key map (kbd "M-i") #'ac-clang-cdb-mode-feature-visit-to-include)
    ;; (define-key map (kbd "M-I") #'ac-clang-cdb-mode-feature-return-from-include)
    (define-key map (kbd "M-[") #'ac-clang-cdb-mode-feature-flymake-goto-prev-error)
    (define-key map (kbd "M-]") #'ac-clang-cdb-mode-feature-flymake-goto-next-error)
    (define-key map (kbd "<f5>") #'ac-clang-cdb-mode-feature-manually-flymake)
    ;; (define-key map (kbd "<C-f5>") #'ac-clang-cdb-mode-feature-build-solution)
    ;; (define-key map (kbd "<f6>") #'ac-clang-cdb-mode-feature-manually-ac-clang-complete)
    ;; (define-key map (kbd "<f7>") #'semantic-force-refresh)
    ;; (define-key map (kbd "C-j") #'ac-clang-cdb-mode-feature-jump-to-project-buffer)
    ;; (define-key map (kbd "C-j") #'ac-clang-cdb-mode-feature-launch-msvs)
    map)
  "ac-clang-cdb mode key map")


(cl-defun ac-clang-cdb--update-mode-line (&optional (keyword ""))
  (setq ac-clang-cdb--mode-line keyword)
  (force-mode-line-update))


(define-minor-mode ac-clang-cdb-mode
  "ac-clang Compilation-Database Mode"
  ;; :lighter ac-clang-cdb--mode-line
  :lighter " ClangCDB"
  :keymap ac-clang-cdb--mode-key-map
  :group 'ac-clang-cdb
  ;; :variable ac-clang-cdb-mode
  (if ac-clang-cdb-mode
      (progn
        ;; (message "ac-clang-cdb-mode : arg : %S" arg)
        ;; (message "ac-clang-cdb-mode : ac-clang-cdb-mode : %S" ac-clang-cdb-mode)
        (if (ac-clang-cdb--evaluate-buffer (eq arg 'auto-active))
            (message "ac-clang-cdb-mode : eval pass")
          (progn
            (message "ac-clang-cdb : This buffer don't belonging to the active projects.")
            (ac-clang-cdb-mode-off)
            )))
    (progn
      (ac-clang-cdb--detach-from-project))))


(defun ac-clang-cdb-mode-on ()
  (interactive)
  (ac-clang-cdb-mode 1))

(defun ac-clang-cdb-mode-off ()
  (interactive)
  (ac-clang-cdb-mode 0))

(defun ac-clang-cdb-mode-auto ()
  (interactive)
  (ac-clang-cdb-mode 'auto-active))



(provide 'ac-clang-cdb)

;; Local Variables:
;; coding: utf-8
;; indent-tabs-mode: nil
;; End:

;;; ac-clang-cdb.el ends here
