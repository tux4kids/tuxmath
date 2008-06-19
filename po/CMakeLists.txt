# The following lists the translations that require SDL_Pango to
# display properly; if we don't have SDL_Pango support, the text is
# blank, which is worse than displaying the English. So if we don't
# have Pango, we want to exclude those translations.
set(TUXMATH_PO_REQUIRES_PANGO
  ar.po
  he.po
  zh*.po
  ru.po
  ja.po
)

# Some translations may be so complete that they should be excluded
# for now. List them here.
set (TUXMATH_EXCLUDE_PO
  fi.po
)

file (GLOB PO_FILES *.po)
file (GLOB PANGO_FILES ${TUXMATH_PO_REQUIRES_PANGO})
file (GLOB EXCLUDE_FILES ${TUXMATH_EXCLUDE_PO})

list(REMOVE_ITEM PO_FILES ${EXCLUDE_FILES})
if(NOT SDLPANGO_FOUND)
  list(REMOVE_ITEM PO_FILES ${PANGO_FILES})
endif(NOT SDLPANGO_FOUND)

add_translations (tuxmath ${PO_FILES})
