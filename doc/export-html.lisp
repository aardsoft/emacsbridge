(progn
  (setq org-html-doctype "html5")
  (eval-after-load "org"
                   '(org-link-set-parameters "export"
                     :follow #'browse-url
                     :export #'(lambda(path desc &optional format)
                                 (format "<a href=\"%s\">%s</a>" path (or desc path)))))
  (load-library "htmlize"))
