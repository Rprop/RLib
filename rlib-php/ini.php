<?php


//### file ###

define('INI_PATH_FUNCTION', $_SERVER['DOCUMENT_ROOT'] . '/functions.php');//函数库路径, 自行修改
define('INI_PATH_SQL_ERROR', $_SERVER['DOCUMENT_ROOT'] . '/error_sql');//查询错误记录文件路径和文件名, 自行修改


//### Mysql ###

define('INI_DB_ADDRESS', '数据库的地址');//数据库地址
define('INI_DB_USERNAME', '数据库的用户名');//mysql用户名
define('INI_DB_PASSWORD', '数据库密码');//mysql密码
define('INI_DEFAULT_DBNAME', '默认选择的数据库名');//默认数据库名
define('INI_CHARSET', 'utf8');//默认编码, 自行修改


//### Others ###

define('INI_ASTARTIME', '1420905600');//agame开始的时间戳


?>