;; emacsbridge-bbdb.el
;; (c) 2020 Bernd Wachter <bwachter-github@aardsoft.fi>

(require 'emacsbridge-rpc)

(defun emacsbridge--bbdb-filter-record (record)
  ""
  (let*
      ((firstname (bbdb-record-firstname record))
       (lastname (bbdb-record-lastname record))
       (organization (bbdb-record-organization record))
       (phone (bbdb-record-phone record))
       (mail (bbdb-record-mail record))
       (filtered-record
        `((firstname . ,firstname)
          (lastname . ,lastname)
          )))
    (bbdb-record-organization record)
    (when organization
      (setq filtered-record (cons `(organization . ,organization) filtered-record)))
    (when (> (length phone) 0)
      (let ((phone-list))
        (dolist (p phone)
          (let* ((phone-record
                  `((number . ,(bbdb-phone-string p))
                    (number-html . ,(concat "<a href=\"tel:"
                                            (bbdb-phone-string p) "\">"
                                            (bbdb-phone-string p) "</a>"))
                    (label . ,(bbdb-phone-label p)))))
            (setq phone-list (cons phone-record phone-list))))
        (setq filtered-record
              (cons `(phones . ,phone-list) filtered-record))))
    ;; TODO: also loop over addresses and mail addresses
    filtered-record
  ))

         (json-str (if (and (boundp requester-id) requester-id)

(defun emacsbridge--bbdb-search-to-json (re &optional requester-id)
  ""
  (let* ((search-result (bbdb-search (bbdb-records) :all-names re))
         (filtered-result (mapcar
                           'emacsbridge--bbdb-filter-record
                           search-result))
         (json-str (if requester-id
                       (json-encode (cons `(:requester-id . ,requester-id) `(:results . ,filtered-result)))
                     (json-encode filtered-result))))
    json-str))
