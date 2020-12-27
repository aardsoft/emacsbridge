;; emacsbridge-rpc.el
;; (c) 2020 Bernd Wachter <bwachter@aardsoft.fi>

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
