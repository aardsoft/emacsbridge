;; emacsbridge.el
;; (c) 2020 Bernd Wachter <bwachter-github@aardsoft.fi>

(defgroup emacsbridge nil
  "emacs bridge related settings"
  :group 'emacsbridge)

(defcustom emacsbridge-default-qml-path nil
  "The default path to search for QML files for uploading to the Android application.
"
  :type 'sexp
  :group 'emacsbridge)

(defcustom emacsbridge-init-file nil
  "The complete path to the file with init code for Emacs bridge"
  :type 'string
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

(defun emacsbridge-init ()
  "Download and configure all files required for Emacs to talk to Emacs bridge"
  )

(provide 'emacsbridge)
