<?php

#include(dirname(__FILE__) . "/../db/mysql.php");

class Log {
	static private $config  =   array(
			'log_time_format'   =>  ' c ',
			'log_file_size'     =>  268435456,
			'log_name'			=>  'alipay',
			'log_path'          =>  '/dsserver/',
	);
	
	static public function record($log,$destination='') {
		$now = date(self::$config['log_time_format']);
		if(empty($destination)){
			$destination = self::$config['log_path'].self::$config['log_name'].'.log';
		}
		// 自动创建日志目录
		$log_dir = dirname($destination);
		if (!is_dir($log_dir)) {
			mkdir($log_dir, 0755, true);
		}
		//检测日志文件大小，超过配置大小则备份日志文件重新生成
		if(is_file($destination) && floor(self::$config['log_file_size']) <= filesize($destination) ){
			rename($destination,dirname($destination).'/'.self::$config['log_name'].'-'.date('Y-m-d-G-i-s').'.log');
		}
		
		error_log("[{$now}] "."{$log}\r\n", 3,$destination);
	}
};

//Log::record("lalala");
//showMysqlInfo();
?>
