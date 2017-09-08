#!/bin/bash

[ -z "$MYSQL_USER" ] && MYSQL_USER=fs
[ -z "$MYSQL_PASSWORD" ] && MYSQL_PASSWORD=fs
[ -z "$MYSQL_DB" ] && MYSQL_DB=fs

exec_mysql()
{
	mysql -u "$MYSQL_USER" -p"$MYSQL_PASSWORD" "$MYSQL_DB"
}

exec_mysql < "$(dirname $0)/drop.sql"
sed 's/"\([a-z]*\)"/`\1`/' "$(dirname $0)/schema.sql" | exec_mysql
exec_mysql < "$(dirname $0)/data.sql"
