#!/usr/bin/env python
# -*- coding: utf-8 -*-

' a test module '

__author__ = 'Yang Chuan'

import sys
import os
import traceback
import mysql.connector
import log.log as log

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
        self.engine = self.create_engine(user, password, database, host, port, **kw)

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

class connection(object):

    def __init__(self, conn):
        self.conn = conn


class autoConnection(connection):
    def __init__(self, conn):
        super(autoConnection, self).__init__(conn)

class transConnection(connection):
    def __init__(self, conn):
        super(transConnection, self).__init__(conn)

class mysqldbc(object):
    CR_SERVER_TIMEOUT = -1  # Error -1 (MySQL Connection not available.)
    CR_SERVER_LOST = 2013
    CR_SERVER_GONE_ERROR = 2006
    CR_SERVER_LOST_EXTENDED = 2055

    def __init__(self, user, password, database, host='127.0.0.1', port=3306, **kw):
        self.connection = None
        self._cursor = None
        self.transactions = 0
        self.engine = engine(user, password, database, host, port, **kw)
        self.default_autocommit = kw['autocommit'] if 'autocommit' in kw else False
        self.current_autocommit = self.default_autocommit

    def mysql_connect(self):
        if self.connection is None:
            try:
                self.connection = self.engine()
                log.Info("connect to database ip:%s, port:%d successful!" % (self.engine.host, self.engine.port))
                #self.transactions = 0
                commit = self.current_autocommit
                self.current_autocommit = self.default_autocommit
                self.__setautocommit(commit)
            except Exception, e:
                log.Error("Failed to connect to database ip:%s, port:%d, Error:(%s)%s"
                          % (self.engine.host, self.engine.port, e.errno, e.msg))
                self.connection = None
                return None
        return self.connection

    def mysql_close(self):
        try:
            self.cursor_close()
            if self.connection:
                self.connection.close()
                self.connection = None
            log.Info("close conneciton from database ip:%s, port:%d !" % (self.engine.host, self.engine.port))
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

    # 这是python mysql的要求(MySQL Connection not available).查看文章https://blog.csdn.net/Money_Bear/article/details/14646289
    # https://blog.csdn.net/jasonjwl/article/details/52702002
    # You must fetch all rows before being able to execute new queries using the same connection.
    # 一个cursor在关闭之前必须要获取完所有的数据集
    def cursor_fetch_all(self, cursor):
        while cursor.fetchone():
            pass

    def cursor_close(self):
        try:
            if self._cursor:
                self.cursor_fetch_all(self._cursor)
                self._cursor.close()
                self._cursor = None
        except Exception as e:
            log.Error("%s" % (traceback.format_exc(),))
            # 可能会触发2055错误，因为connection感知不到mysql停止

    def begin(self):
        # autocommit=False,
        if not self.mysql_connect():
            return False
        self.__setautocommit(False)
        self.addTransactions()
        return True

    def commit(self):
        try:
            self.delTransactions()
            if self.current_autocommit is False and not self.in_transactions():
                if self.connection:
                    self.connection.commit()
                self.__setautocommit(self.default_autocommit)
            return True
        except Exception as e:
            log.Error("%s" % (traceback.format_exc(),))
            # 可能会触发2055错误，因为connection感知不到mysql停止，rollback一般在except中，所以不用处理异常
            return False

    def rollback(self):
        try:
            self.delTransactions()
            if self.current_autocommit is False and not self.in_transactions():
                if self.connection:
                    self.connection.rollback()
                self.__setautocommit(self.default_autocommit)
            return True
        except Exception as e:
            log.Error("%s" % (traceback.format_exc(),))
            # 可能会触发2055错误，因为connection感知不到mysql停止，rollback一般在except中，所以不用处理异常
            return False

    def __setautocommit(self, autocommit=False):
        if self.current_autocommit == autocommit:
            return True
        try:
            self.current_autocommit = autocommit
            if self.connection:
                self.connection.autocommit = autocommit
            return True
        except Exception, e:
            log.Error("%s" % (traceback.format_exc(),))
            log.Error("mysqldbc.select mysql_query Error %d (%s): '%s'" % (e.errno, e.msg, sql))

            return False


    def addTransactions(self):
        self.transactions = self.transactions + 1

    def delTransactions(self):
        if self.transactions > 0:
            self.transactions = self.transactions - 1

    def in_transactions(self):
        return True if self.transactions > 0 else False

    def reconnect(self):
        self.mysql_close()
        self.mysql_connect()
        return True

    def select(self, sql):
        try:
            self.addTransactions()
            cursor = self.cursor()
            cursor.execute(sql)
            result = [x for x in cursor.fetchall()]
            self.commit()
            return result
        except AttributeError, e:
            log.Error("%s" % (traceback.format_exc(),))
            log.Error("mysqldbc.select failed: sql: %s" % sql)  # cursor maybe None
            self.rollback()
            return None
        except Exception, e:
            log.Error("%s" % (traceback.format_exc(),))
            log.Error("mysqldbc.select mysql_query Error %d (%s): '%s'" % (e.errno, e.msg, sql))
            self.rollback()
            if (e.errno == self.CR_SERVER_TIMEOUT or e.errno == self.CR_SERVER_GONE_ERROR or e.errno == self.CR_SERVER_LOST or e.errno == self.CR_SERVER_LOST_EXTENDED):
                self.mysql_close()
                if not self.in_transactions():
                    # 非事务可以重新执行一次
                    log.Info("self.select again")
                    return self.select(sql)
            return None
        finally:
            self.cursor_close()

    def insert(self, sql):
        try:
            self.addTransactions()
            cursor = self.cursor()
            cursor.execute(sql)
            self.commit()
            return cursor.lastrowid
        except AttributeError, e:
            log.Error("%s" % (traceback.format_exc(),))
            log.Error("mysqldbc.insert failed: sql: %s" % sql)
            self.rollback()
            return None
        except Exception, e:
            log.Error("%s" % (traceback.format_exc(),))
            log.Error("mysqldbc.insert mysql_query Error %d (%s): '%s'" % (e.errno, e.msg, sql))
            self.rollback()
            if (e.errno == self.CR_SERVER_TIMEOUT or e.errno == self.CR_SERVER_GONE_ERROR or e.errno == self.CR_SERVER_LOST or e.errno == self.CR_SERVER_LOST_EXTENDED):
                self.mysql_close()
                if not self.in_transactions():
                    #非事务可以重新执行一次
                    log.Info("self.insert again")
                    return self.insert(sql)
            return None
        finally:
            self.cursor_close()

    def update(self, sql):
        try:
            self.addTransactions()
            cursor = self.cursor()
            cursor.execute(sql)
            self.commit()
            return cursor.rowcount
        except AttributeError, e:
            log.Error("mysqldbc.update failed: sql: %s" % sql)
            self.rollback()
            return None
        except Exception, e:
            log.Error("%s" % (traceback.format_exc(),))
            log.Error("mysqldbc.update mysql_query Error %d (%s): '%s'" % (e.errno, e.msg, sql))
            self.rollback()
            if (e.errno == self.CR_SERVER_TIMEOUT or e.errno == self.CR_SERVER_GONE_ERROR or e.errno == self.CR_SERVER_LOST or e.errno == self.CR_SERVER_LOST_EXTENDED):
                self.mysql_close()
                if not self.in_transactions():
                    #非事务可以重新执行一次
                    log.Info("self.update again")
                    return self.update(sql)
            return None
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

    def info(self):
        log.Debug("transactions: %s, current_autocommit: %s default_autocommit: %s" % (self.transactions, self.current_autocommit, self.default_autocommit))

def do_test(db):
    pass


if __name__ == '__main__':
    log.LogInit(level="debug")
    # test()
    # do_insert = False

    mysqldb = mysqldbc("ijmaster", 'ijjazhang', 'sync_ijdbs', host='127.0.0.1', port=3306)

    mysqldb.select_test()

    mysqldb.close()