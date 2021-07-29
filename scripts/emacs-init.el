;; emacs-init.el
;; (c) 2021 Bernd Wachter <bwachter-github@aardsoft.fi>
;;
;; to work on this without going through Emacs bridge just evaluate
;; some or all of the following expressions:
;;
;; (setq {{http/bindPort}} "1616")
;; (setq {{http/bindAddress}} "127.0.0.1")
;;
;; (eval-buffer)

(let ((expected-server-port "")
      (expected-server-address "")
      (expected-server-use-tcp "")
(progn
  (switch-to-buffer (url-retrieve-synchronously
                     (concat "http://"
                             {{http/bindAddress}} ":" {{http/bindPort}}
                             "/local/lisp/emacsbridge.el")))
  (delete-region (point-min) url-http-end-of-headers)
  (eval-buffer)
  (kill-buffer (current-buffer)))
