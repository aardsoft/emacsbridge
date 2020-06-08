;; alert-emacsbridge.el
;; (c) 2020 Bernd Wachter <bwachter@aardsoft.fi>

(require 'json)
(require 'emacsbridge-rpc)
(require 'alert)

(defcustom emacsbridge-fallback-notifier nil
  "If notification fails (usually because the application is not running), fall
back to the notifier specified here."
  :type (alert-styles-radio-type 'radio)
  :group 'emacsbridge)

(defun alert-emacsbridge-notify(info)
  "Send INFO using emacs bridge.
Handles .."
  (let* ((json-str
          (json-encode `((title . ,(alert-encode-string (plist-get info :title)))
                         (text . ,(alert-encode-string (plist-get info :message)))))
          ))
    (message json-str)
    (condition-case e
        (emacsbridge-post-json "notification" json-str)
      (error
       (when emacsbridge-fallback-notifier
         (let ((alert-default-style emacsbridge-fallback-notifier))
           (alert info)))))))

(alert-define-style 'emacsbridge :title "Notify using emacs bridge"
                    :notifier #'alert-emacsbridge-notify)

(provide 'alert-emacsbridge)
;;; alert-emacsbridge.el ends here
