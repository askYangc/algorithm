#!/usr/bin/env python
# -*- coding: utf-8 -*-

' a test module '

__author__ = 'Yang Chuan'

import sys
import os
import traceback
import mysql.connector
import log

reload(sys)
sys.setdefaultencoding('utf-8')


class engine(object):

    def __init__(self, user, password, database, host='127.0.0.1', port=3306, **kw):
        self.user = user
        self.password = password
        self.database = database
        self.host = host
        if isinstance(port, str):
            port = int(port)
        self.port = port
        self.kw = kw
        self.engine = self.create_engine(user, password, database, host, port, *kw)

    def create_engine(self, user, password, database, host='127.0.0.1', port=3306, **kw):
        params = dict(user=user, password=password, database=database, host=host, port=port)
        defaults = dict(use_unicode=True, charset='utf8', collation='utf8_general_ci', autocommit=False)
        for k, v in defaults.iteritems():
            params[k] = kw.pop(k, v)
        params.update(kw)
        params['buffered'] = True
        engine = lambda: mysql.connector.connect(**params)
        # test connection...
        # log.Info('Init mysql engine <%s> ok.' % hex(id(engine)))
        return engine

    def __call__(self, *args, **kwargs):
        return self.engine()


class mysqldbc(object):
    CR_SERVER_TIMEOUT = -1  # Error -1 (MySQL Connection not available.)
    CR_SERVER_LOST = 2013
    CR_SERVER_GONE_ERROR = 2006
    CR_SERVER_LOST_EXTENDED = 2055
    MAX_CON = 1

    def __init__(self, user, password, database, host='127.0.0.1', port=3306, **kw):
        self.connection = None
        self._cursor = None
        self.retry_connection = 0
        self.transactions = 0
        self.engine = engine(user, password, database, host, port, *kw)

    def mysql_connect(self):
        if self.connection is None:
            try:
                self.connection = self.engine()
                log.Info("connect to database ip:%s, port:%d successful!" % (self.engine.host, self.engine.port))
                self.retry_connection = 0
                self.transactions = 0
            except Exception, e:
                log.Error("Failed to connect to database ip:%s, port:%d, Error:(%s)%s"
                          % (self.engine.host, self.engine.port, e.errno, e.msg))
                self.connection = None
                return None
        return self.connection

    def mysql_close(self):
        try:
            if self._cursor:
                self._cursor.close()
                self._cursor = None

            if self.connection:
                self.connection.close()
                self.connection = None
        except Exception as e:
            log.Error("%s" % (traceback.format_exc(),))
        finally:
            self._cursor = None
            self.connection = None

    def cursor(self):
        if not self.mysql_connect():
            return None
        if self._cursor is None:
            self._cursor = self.connection.cursor()
        return self._cursor

    def cursor_close(self):
        try:
            if self._cursor:
                self.cursor_fetch_all(self._cursor)
                self._cursor.close()
                self._cursor = None
        except Exception as e:
            log.Error("%s" % (traceback.format_exc(),))
            # 可能会触发2055错误，因为connection感知不到mysql停止

    def commit(self):
        try:
            self.delTransactions()
            if self.connection and self.transactions == 0:
                self.connection.commit()
            return True
        except Exception as e:
            log.Error("%s" % (traceback.format_exc(),))
            # 可能会触发2055错误，因为connection感知不到mysql停止，rollback一般在except中，所以不用处理异常
            return False

    def rollback(self):
        try:
            self.delTransactions()
            if self.connection and self.transactions == 0:
                self.connection.rollback()
            return True
        except Exception as e:
            log.Error("%s" % (traceback.format_exc(),))
            # 可能会触发2055错误，因为connection感知不到mysql停止，rollback一般在except中，所以不用处理异常
            return False

    def addTransactions(self):
        self.transactions = self.transactions + 1

    def delTransactions(self):
        if self.transactions > 0:
            self.transactions = self.transactions - 1

    def in_transactions(self):
        return True if self.transactions > 0 else False

    def begin(self):
        # autocommit=False,
        if not self.mysql_connect():
            return False
        self.addTransactions()
        return True

    def reconnect(self):
        if self.retry_connection >= self.MAX_CON:
            self.retry_connection = 0
            return False
        self.mysql_close()
        self.retry_connection = self.retry_connection + 1
        return True

    def select(self, sql):
        try:
            cursor = self.cursor()
            cursor.execute(sql)
            result = [x for x in cursor.fetchall()]
            return result
        except AttributeError, e:
            log.Error("%s" % (traceback.format_exc(),))
            log.Error("mysqldbc.select failed: sql: %s" % sql)  # cursor maybe None
            if self.in_transactions():
                return None
            return []
        except Exception, e:
            log.Error("%s" % (traceback.format_exc(),))
            log.Error("mysqldbc.select mysql_query Error %d (%s): '%s'" % (e.errno, e.msg, sql))
            if (e.errno == self.CR_SERVER_TIMEOUT or e.errno == self.CR_SERVER_GONE_ERROR or e.errno == self.CR_SERVER_LOST or e.errno == self.CR_SERVER_LOST_EXTENDED) and self.reconnect():
                return self.select(sql)
            self.mysql_close()
            if self.in_transactions():
                return None
            return []
        finally:
            self.cursor_close()

    # 这是python mysql的要求(MySQL Connection not available).查看文章https://blog.csdn.net/Money_Bear/article/details/14646289
    # https://blog.csdn.net/jasonjwl/article/details/52702002
    # You must fetch all rows before being able to execute new queries using the same connection.
    # 一个cursor在关闭之前必须要获取完所有的数据集
    def cursor_fetch_all(self, cursor):
        while cursor.fetchone():
            pass

    def insert(self, sql):
        try:
            cursor = self.cursor()
            self.addTransactions()
            cursor.execute(sql)
            self.commit()
            return cursor.lastrowid
        except AttributeError, e:
            log.Error("%s" % (traceback.format_exc(),))
            log.Error("mysqldbc.insert failed: sql: %s" % sql)
            self.rollback()
            return -1
        except Exception, e:
            log.Error("%s" % (traceback.format_exc(),))
            log.Error("mysqldbc.insert mysql_query Error %d (%s): '%s'" % (e.errno, e.msg, sql))

            self.rollback()
            if (e.errno == self.CR_SERVER_TIMEOUT or e.errno == self.CR_SERVER_GONE_ERROR or e.errno == self.CR_SERVER_LOST or e.errno == self.CR_SERVER_LOST_EXTENDED) and self.reconnect():
                return self.insert(sql)
            self.mysql_close()
            return -1
        finally:
            self.cursor_close()

    def update(self, sql):
        try:
            cursor = self.cursor()
            self.addTransactions()
            cursor.execute(sql)
            self.commit()
            return cursor.rowcount
        except AttributeError, e:
            log.Error("mysqldbc.update failed: sql: %s" % sql)
            self.rollback()
            return -1
        except Exception, e:
            log.Error("%s" % (traceback.format_exc(),))
            log.Error("mysqldbc.update mysql_query Error %d (%s): '%s'" % (e.errno, e.msg, sql))
            self.rollback()
            if (e.errno == self.CR_SERVER_TIMEOUT or e.errno == self.CR_SERVER_GONE_ERROR or e.errno == self.CR_SERVER_LOST or e.errno == self.CR_SERVER_LOST_EXTENDED) and self.reconnect():
                return self.update(sql)
            self.mysql_close()
            return -1
        finally:
            self.cursor_close()

    def close(self):
        self.mysql_close()

    def select_test(self):
        sql = 'select * from test'
        print self.select(sql)

    def insert_test(self):
        sql = 'insert into test(name,num) value("yangchuan", 1)'
        print self.insert(sql)

def do_test(db):
    pass


if __name__ == '__main__':
    log.LogInit(level="debug")
    # test()
    # do_insert = False

    mysqldb = mysqldbc("ijmaster", 'ijjazhang', 'sync_ijdbs', host='127.0.0.1', port=3306)

    mysqldb.select_test()

    mysqldb.close()