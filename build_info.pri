HEAD_REV=$$system(git rev-parse --short HEAD, blob, ret)
equals(ret, 0){
  message("Building from git")
  NEWEST_TAG=$$system(git describe --abbrev=0 --tags)
  NEWEST_TAG_DIRTY=$$system(git describe --abbrev=0 --tags --dirty="/dirty")
  COMMIT_COUNT=$$system(git rev-list $${NEWEST_TAG}.. --count)
  TOTAL_COMMIT_COUNT=$$system(git rev-list HEAD --count)
}else{
  NEWEST_TAG_DIRTY=""
  include(release_version.pri)
  message("Building from tarball")
}

DIRTY=$$split(NEWEST_TAG_DIRTY, /)
message("Newest tag: $$NEWEST_TAG")
message("Commits since tag: $$COMMIT_COUNT")
message("Total commits: $$TOTAL_COMMIT_COUNT")
contains(DIRTY, dirty){
  VERSION=$${NEWEST_TAG}.$${COMMIT_COUNT}-dirty
}else{
  VERSION=$${NEWEST_TAG}.$${COMMIT_COUNT}
}
message("Building version $$VERSION")

DEFINES += VERSION="'\"$$VERSION\"'"