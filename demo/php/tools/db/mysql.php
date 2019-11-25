<?php

class mysqlConn {
	protected $conn = NULL;
	protected $host = DB_HOST;
	protected $user = DB_USER;
	protected $passwd = DB_PASSWORD;
	protected $port = DB_PORT;
	protected $dbname = DB_NAME;
	protected $ssl_ca = SSL_CA;
	protected $ssl_cert = SSL_CERT;
	protected $ssl_key = SSL_KEY;
	protected $ssl_enable = SSL_ENABLE;

	function __construct(){
		$a = func_get_args();
		$i = func_num_args();
		
		if($i == 0) {
			$this->init();
		}else {
			return call_user_func_array(array($this,"init"), $a);
		}
	}
	
	function __destruct() {
		$this->close();
	}
	
	public function showInfo() {
		echo "host $this->host, port $this->port, user $this->user, passwd $this->passwd, dbname $this->dbname\n";
	}
	
	protected function init($host=DB_HOST, $user=DB_USER, $passwd=DB_PASSWORD, $port=DB_PORT, $dbname=DB_NAME){
		$this->host = $host;
		$this->user = $user;
		$this->passwd = $passwd;
		$this->port = $port;
		$this->dbname = $dbname;
		
		$this->conn = $this->mysql_init();
		if($this->conn == NULL) {
			Log::record("connect $this->host:$this->port failed\n");
			return false;
		}
		return true;
	}
	
	protected function mysql_init() {
		$error_level = error_reporting(0);
		
		//use ssl
		$conn = mysqli_init();
		if($this->ssl_enable) {
			$conn->ssl_set($this->ssl_key, $this->ssl_cert, $this->ssl_ca, NULL, NULL);
		}
		
		$result = $conn->real_connect($this->host, $this->user, $this->passwd, 
				$this->dbname, $this->port, NULL, MYSQLI_CLIENT_SSL_DONT_VERIFY_SERVER_CERT);
		//$conn = new mysqli($this->host, $this->user, $this->passwd, $this->dbname, $this->port);
		error_reporting($error_level);
		if($result == false || $conn->connect_error) {
			Log::record("连接失败: " . $conn->connect_error);
			return null;
		}
		mysqli_set_charset($conn, "utf8");
		Log::record("mysql 连接成功 " . $conn->host_info . "\n");
		return $conn;
	}
	
	public function begin() {
		#$sql = 'begin';
		#$this->execute($sql);
		$this->conn->autocommit(FALSE);
	}
	
	public function commit() {
		#$sql = 'commit';
		#$this->execute($sql);
		$this->conn->commit();
		$this->conn->autocommit(TRUE);
	}

	public function rollback() {
		#$sql = 'rollback';
		#$this->execute($sql);
		$this->conn->rollback ();
		$this->conn->autocommit(TRUE);
	}
	
	//直接返回result，可以通过$result->num_rows得到个数，
	//通过$result->fetch_row()得到每行数据(fetch_row会生成数组{数字索引=>值})
	//通过$result->fetch_assoc()得到每行数据(fetch_row会生成数组{列名=>值})
	//通过$result->fetch_array()得到每行数据(fetch_row会生成数组{数字索引=>值和列名=>值})
	//$result->lengths得到每列的长度(需要先执行fetch_row更新lengths)
	//while ($row = $result->fetch_row()) {}
	//while ($row = $result->fetch_array()) {}
	//while ($row = $result->fetch_assoc()) {}
	public function query($sql) {
		$result = $this->conn->query($sql);
		if($result == True) {
			return $result;
		}
		
		$err = $this->conn->errno;
		$errstring = $this->conn->error;
		Log::record("query_array Error $sql, $err: $errstring\n");
		if($this->conn->errno == 2013 || $this->conn->errno == 2006) {
			//connect again
			$this->close();
			if(($this->conn = $this->mysql_init()) != NULL) {
				Log::record("Again connect mysql database unsuccessful!\n");
				$result = $this->conn->query($sql);
				if($result == True)	{
					return $result;
				}else {
					return False;
				}
			}
		}
		
		return False;
	}
	
	//MYSQLI_ASSOC 列字段为key，数据为value
	//MYSQLI_NUM  key为0,1,2 数据为value
	//MYSQLI_BOTH 既有列字段，又有数字索引
	public function query_array($sql, $type=MYSQLI_ASSOC)
	{
		$result = $this->conn->query($sql);
		if($result == True) {
			return $result->fetch_all($type);
		}
		
		$err = $this->conn->errno;
		$errstring = $this->conn->error;
		Log::record("query_array Error $sql, $err: $errstring\n");
		if($this->conn->errno == 2013 || $this->conn->errno == 2006) {
			//connect again
			$this->close();
			if(($this->conn = $this->mysql_init()) != NULL) {
				Log::record("Again connect mysql database unsuccessful!\n");
				$result = $this->conn->query($sql);
				if($result == True)	{
					return $result->fetch_all($type);
				}else {
					return NULL;
				}
			}
		}
		
		return NULL;
	}
	
	//MYSQLI_ASSOC 列字段为key，数据为value
	//MYSQLI_NUM  key为0,1,2 数据为value
	//MYSQLI_BOTH 既有列字段，又有数字索引
	public function query_array_one($sql, $type=MYSQLI_ASSOC)
	{
		$result = $this->conn->query($sql);
		if($result == True) {
			return $result->fetch_array($type);
		}
		
		$err = $this->conn->errno;
		$errstring = $this->conn->error;
		Log::record("query_array_one Error $sql, $err: $errstring\n");
		if($this->conn->errno == 2013 || $this->conn->errno == 2006) {
			//connect again
			$this->close();
			if(($this->conn = $this->mysql_init()) != NULL) {
				Log::record("Again connect mysql database unsuccessful!\n");
				$result = $this->conn->query($sql);
				if($result == True)	{
					return $result->fetch_array($type);
				}else {
					return NULL;
				}
			}
		}
		
		return NULL;
	}
	
	//after select/update/insert/delete
	public function affected_rows()
	{
		return $this->conn->affected_rows;
	}
	
	public function execute($sql)
	{
		if($this->conn->query($sql) === True) {
			return True;
		}else {
			$err = $this->conn->errno;
			$errstring = $this->conn->error;
			Log::record("execute Error $sql, $err: $errstring\n");
			if($this->conn->errno == 2013 || $this->conn->errno == 2006) {
				//connect again
				$this->close($conn);
				if(($this->conn = $this->mysql_init()) != NULL) {
					Log::record("Again connect mysql database unsuccessful!\n");
					if($this->conn->query($sql) === True) {
						return True;
					}else {
						return False;
					}
				}
			}
			
			return False;
		}
	}
	
	function close()
	{
		if($this->conn) {
			$this->conn->close();
			$this->conn = NULL;
		}
		return ;
	}
};

function showMysqlInfo()
{
	echo "DB_HOST: ".DB_HOST.PHP_EOL;
	echo "DB_USER: ".DB_USER.PHP_EOL;
	echo "DB_PASSWORD: ".DB_PASSWORD.PHP_EOL;
	echo "DB_PORT: ".DB_PORT.PHP_EOL;
	echo "DB_NAME: ".DB_NAME.PHP_EOL;
}

function  init()
{
	//define('DB_HOST', "10.135.255.201", false);
	//define('DB_USER', "ijmaster", false);
	//define('DB_PASSWORD', "ijjazhang", false);
	//define('DB_PORT', 3306, false);
	//define('DB_NAME', "sync_ijdbs", false);
	
	//读取配置文件
	$conf = '/dsserver/dsserver.conf';
	$c = parse_ini_file($conf);
	define('DB_HOST', isset($c['sync_db_ip']) ? $c['sync_db_ip']:"10.135.255.201", false);
	define('DB_USER', isset($c['sync_db_user']) ? $c['sync_db_user']:"ijmaster", false);
	define('DB_PASSWORD', isset($c['sync_db_pass']) ? $c['sync_db_pass']:"ijjazhang", false);
	define('DB_PORT', isset($c['sync_db_port']) ? intval($c['sync_db_port']):3306, false);
	define('DB_NAME', isset($c['sync_db_name']) ? $c['sync_db_name']:"sync_ijdbs", false);
	
	//define('SSL_ENABLE', isset($c['ssl-enable']) ? $c['ssl-enable']:0, false);
	define('SSL_ENABLE', 1, false);
	define('SSL_CA', isset($c['ssl-ca']) ? $c['ssl-ca']:"/usr/local/mysql/mysql-test/std_data/cacert.pem", false);
	define('SSL_CERT', isset($c['ssl-clicert']) ? $c['ssl-clicert']:"/usr/local/mysql/mysql-test/std_data/client-cert.pem", false);
	define('SSL_KEY', isset($c['ssl-clikey']) ? $c['ssl-clikey']:"/usr/local/mysql/mysql-test/std_data/client-key.pem", false);
}

function mysql_connect_test()
{
	$conn = new mysqlConn();
	$sql = "select * from test";
	$r = $conn->query_array($sql);
	if($r == NULL) {
		echo "not result\n";
	}
	
	var_dump($r);
	
	foreach($r as $k => $v) {
		echo "---start----\n";
		var_dump($k);
		var_dump($v);
		echo "---end----\n";
	}
}


init();
//mysql_connect_test();


?>
