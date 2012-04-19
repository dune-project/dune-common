
# Somehow variable list get destroyed when calling cmake (; is replaced with
# whitespace character. Undo this change
string(REGEX REPLACE "([a-zA-Z0-9]) ([/a-zA-Z0-9])" "\\1;\\2" files "${FILES}")
file(INSTALL ${files} DESTINATION ${DIR})

