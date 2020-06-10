;; emacsbridge-rpc.el
;; (c) 2020 Bernd Wachter <bwachter@aardsoft.fi>

(defgroup emacsbridge nil
  "emacs bridge related settings"
  :group 'emacsbridge)

(defcustom emacsbridge-default-qml-path nil
  "The default path to search for QML files for uploading to the Android application.
"
  :type 'sexp
  :group 'emacsbridge)

(defcustom emacsbridge-config-from-emacs t
  "Controls if Emacs sets and possibly overrides the configuration of the Android application. Currently the only sensible value is t.
Even if the Android application gains a usable settings UI keeping the configuration in Emacs makes it easier to switch phones."
  :type 'sexp
  :group 'emacsbridge)

(defcustom emacsbridge-auth-token nil
  "The authentication token used for accessing the RPC interface. This is automatically configured during the initial setup."
  :type 'string
  :group 'emacsbridge)

(defcustom emacsbridge-server-port "1616"
  "The port where the emacs bridge server is listening. If this needs changing
it usually should be handled by the server on connect"
  :type 'string
  :group 'emacsbridge)

(defcustom emacsbridge-server-host "127.0.0.1"
  "The host where the emacs bridge server is listening. It generally is a bad
idea to not have this at the default value."
  :type 'string
  :group 'emacsbridge)

(defun emacsbridge--find-auth-key ()
  "Get the Emacs server auth key, either from server file or from the
`server-auth-key' variable"
  (if (and (boundp 'server-auth-key) server-auth-key)
      server-auth-key
    (progn
      (if (file-exists-p (concat server-auth-dir "/server"))
          (progn
            (with-temp-buffer
              (insert-file-contents (concat server-auth-dir "/server"))
              (cadr (split-string (buffer-string) "\n" t))
              ))))))

(defun emacsbridge-push-auth-key()
  "Push the Emacs server auth key to the Android application. This is generally
required once per session, unless `server-auth-key' is in use"
  (let ((auth-key (emacsbridge--find-auth-key)))
    (if auth-key
        (emacsbridge-post-setting "server-key" auth-key)
      (message "No auth key configured on this system"))))

(defun emacsbridge--find-qml-file(name &optional file)
  "Try to locate a local QML file. The .qml suffix is appended if missing.

Search order is current directory, `emacsbridge-default-qml-path', and ../qml
relative to the library location."
  (let* ((qml-file (or file name))
         (qml-file (if (string-suffix-p ".qml" qml-file)
                       qml-file
                     (concat qml-file ".qml"))))
    (cond ((file-exists-p qml-file)
           (expand-file-name qml-file))
          ((and emacsbridge-default-qml-path
                (file-exists-p (concat emacsbridge-default-qml-path
                                       "/"
                                       qml-file)))
           (concat emacsbridge-default-qml-path
                   "/"
                   qml-file))
          ((and load-file-name
                (file-exists-p (concat (file-name-directory load-file-name)
                                       "../qml/"
                                       qml-file)))
           (concat (file-name-directory load-file-name)
                   "../qml/"
                   qml-file))
          )))

(defun emacsbridge-push-qml(qml)
  "Push a QML page to the Android application"
  (let ((qml-file (emacsbridge--find-qml-file
                   (plist-get qml :name)
                   (plist-get qml :file))))
    (if (and qml-file (file-exists-p qml-file))
        (let* ((qml-data (with-temp-buffer
                           (insert-file-contents qml-file)
                           (buffer-string)))
               (json-str
                (json-encode
                 `((:file-name . ,qml-file)
                   (:in-drawer . ,(plist-get qml :in-drawer))
                   (:title . ,(plist-get qml :title))
                   (:qml-data . ,qml-data)
                   ))))
          (message  json-str)
          (emacsbridge-post-json "addComponent" json-str)))))

(defun emacsbridge-pop-qml(name)
  "Remove a QML page from the Android application"
  (let* ((qml-file (if (string-suffix-p ".qml" name)
                       name
                     (concat name ".qml")))
         (json-str
          (json-encode
           `((:file-name . ,qml-file)))))
    (emacsbridge-post-json "removeComponent" json-str)))

(defun emacsbridge-post-setting (setting value)
  "POST a configuration item to the settings URL of the Emacs bridge application"
  (let ((url-request-method "POST")
        (url-request-extra-headers
         `(("Content-Type" ."x-www-form-urlencoded")
           ("auth-token" . ,emacsbridge-auth-token)
           ("setting" . ,setting)))
        (url-request-data value))
    (condition-case e
        (with-current-buffer (url-retrieve-synchronously
                              (concat "http://" emacsbridge-server-host ":" emacsbridge-server-port "/settings")))
      (file-error
       (if (and (boundp 'emacsbridge--ignore-rpc-errors) emacsbridge--ignore-rpc-errors)
           (message "Emacsbridge: Unable to send RPC query, but ignoring.")
         (error "Request failed: %s" e))))))

(defun emacsbridge-post-json (method json)
  "POST JSON data to the RPC URL of the Emacs bridge application"
  (let ((url-request-method "POST")
        (url-request-extra-headers
         `(("Content-Type" ."application/json")
           ("auth-token" . ,emacsbridge-auth-token)
           ("method" . ,method)))
        (url-request-data (encode-coding-string json 'utf-8)))
    (condition-case e
        (with-current-buffer (url-retrieve-synchronously
                              (concat "http://" emacsbridge-server-host ":" emacsbridge-server-port "/rpc")))
      (file-error
       (if (and (boundp 'emacsbridge--ignore-rpc-errors) emacsbridge--ignore-rpc-errors)
           (message "Emacsbridge: Unable to send RPC query, but ignoring.")
         (error "Request failed: %s" e))))))

(provide 'emacsbridge-rpc)
;;; emacsbridge-rpc.el ends here
