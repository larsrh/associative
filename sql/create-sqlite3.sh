#!/bin/bash

rm -f db.sqlite db.sqlite-journal
sqlite3 db.sqlite < "$(dirname $0)/schema.sql"
sqlite3 db.sqlite < "$(dirname $0)/data.sql"
