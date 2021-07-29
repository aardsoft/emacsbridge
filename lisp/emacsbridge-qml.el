;; emacsbridge-qml.el
;; (c) 2020 Bernd Wachter <bwachter-github@aardsoft.fi>

(require 'emacsbridge-rpc)

(defun emacsbridge-qml--find-file(name &optional file)
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

(defun emacsbridge-qml-push(qml)
  "Push a QML page to the Android application"
  (let ((qml-file (emacsbridge-qml--find-file
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

(defun emacsbridge-qml-pop(name)
  "Remove a QML page from the Android application"
  (let* ((qml-file (if (string-suffix-p ".qml" name)
                       name
                     (concat name ".qml")))
         (json-str
          (json-encode
           `((:file-name . ,qml-file)))))
    (emacsbridge-post-json "removeComponent" json-str)))

(provide 'emacsbridge-qml)
