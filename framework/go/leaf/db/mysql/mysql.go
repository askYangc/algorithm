package mysql

import (
	"database/sql"
	"fmt"
	_ "github.com/go-sql-driver/mysql"
	"leaf/log"
)

func Open(ip string, port uint16, user string, passwd string) {
	s := fmt.Sprintf("%s:@%s(%s:%s)/test?charset=utf8", user, passwd, ip, port)
	db, err := sql.Open("mysql", s)
	if err != nil {
		log.Error("open mysql (%s:%s) user %s pwd %s failed\n", user, passwd, ip, port)
		return nil
	}
}

func Close(db *sql.DB) {
	if db != nil {
		db.Close()
	}
}