# SQLite support
if(ASSOCIATIVE_WITH_SQLITE)
	set(SQLITE_LIBS sqlite3)
else(ASSOCIATIVE_WITH_SQLITE)
	set(SQLITE_LIBS "")
endif(ASSOCIATIVE_WITH_SQLITE)

set(OPT_LIBS ${OPT_LIBS} ${SQLITE_LIBS})