<?php


//### file ###

define('INI_PATH_FUNCTION', $_SERVER['DOCUMENT_ROOT'] . '/rlib-functions.php');//函数库路径和名称(这里为根目录
define('INI_PATH_SQL_ERROR', $_SERVER['DOCUMENT_ROOT'] . '/error_sql');//查询错误记录文件路径和名称(这里为根目录


//### Mysql ###

define('INI_DB_ADDRESS', '************');//数据库地址
define('INI_DB_USERNAME', '************');//mysql用户名
define('INI_DB_PASSWORD', '************');//mysql密码
define('INI_DEFAULT_DBNAME', '************');//默认数据库名
define('INI_CHARSET', 'utf8');//默认编码(这里为utf8


//### Others ###

define('INI_ASTARTIME', '1420905600');//game2开始时间戳

date_default_timezone_set('Asia/Shanghai');//设时区(这里为+8时区
header("Content-type: text/html; charset=utf-8");//页面编码(这里为utf-8

?>