(progn
  (setq org-html-doctype "html5")
  (setq org-html-validation-link nil)
  (eval-after-load "org"
                   '(org-link-set-parameters "export"
                     :follow #'browse-url
                     :export #'(lambda(path desc &optional format)
                                 (format "<a href=\"%s\">%s</a>" path (or desc path)))))
  (load-library "htmlize"))
