<?php
/*
 * 本页面载入的前置是 ini.php
 */





/**
 * 数据库连接
 * 
 * 参数(可选): 
 * $host 数据库地址
 * $username 用户名
 * $passwd 密码
 * $dbname 数据库名
 * 
 * 返回值: 打开的连接(mysql资源)
 * 
 * 2016-02-22
 */ 
function the_mysql_connect($arr = array())
{

    $host = INI_DB_ADDRESS;
    $username = INI_DB_USERNAME;
    $passwd = INI_DB_PASSWORD;
    $dbname = INI_DEFAULT_DBNAME;
    extract($arr);//拆开数组(覆盖)
    
    $con = mysqli_connect($host, $username, $passwd, $dbname) or the_mysql_error(array('message' => "连接错误| {$host}| {$username}| {$passwd}| {$dbname}|"));
    mysqli_set_charset($con, INI_CHARSET);//设置默认字符编码
    
    return $con;
}



/**
 * 数据库查询语句执行
 * 
 * 参数1: 数据库连接
 * 参数2: 查询语句
 * 参数3(可选): 调试开关debug 记录开关note
 * 
 * 一定要记得先转义需要转义的查询值再传
 * 
 * 返回值: 生结果集(结果集对象)
 * 
 * 2016-02-22
 */
function the_mysql_query($con, $str, $arr = array())
{
    $debug = FALSE;
    $note = FALSE;
    extract($arr);//拆开数组(覆盖)

    //$str = mysqli_real_escape_string($con, $str);//转义
    if($debug){echo "[$str]";}//调试用, 直接输出查询的语句
    $res = mysqli_query($con, $str) or the_mysql_error(array('message' => "数据库查询语句执行出错| 查询语句: {$str}| 错误信息: " . mysqli_error($con)));
    mysqli_close($con);
    
    if($note){the_mysql_error(array('message' => $str, 'note' => TRUE));}//记录用
    
    return $res;
}


/**
 * 查询语句执行错误记录
 * (日志默认回车符为Unix的)
 * 
 * 参数(可选): 错误信息message(字符串) 记录note(布尔值)(默认为假, 真则不中断脚本)
 * 
 * 2016-02-22
 */
function the_mysql_error($arr = array())
{
    $message = '木有错误信息';
    $note = FALSE;
    extract($arr);//拆开数组(覆盖)
    
    date_default_timezone_set('Asia/Shanghai');//设时区
    
    $str = "[ERROR]\n";
    if($note){$str = "[NOTE]\n";}

    $str .= date('Y-m-d H:i:s') . '  ip: ' . get_user_ip() . "\n" . $message . "\n\n";

    $handle = fopen(INI_PATH_SQL_ERROR, 'a+');
    fwrite($handle, $str);
    fclose($handle);
    
    if(!$note){die('啊.代码又出bug了..db');}
    
    return TRUE;
    
}




/**
 * mysqli_real_escape_string别名
 * 
 * 参数1: mysql连接
 * 参数2: 转义字符串
 * 
 * 返回值: 转义后的字符串
 * 
 * 2016-02-22
 */
function tosql($con, $str)
{
    return mysqli_real_escape_string($con, $str);
}



/**
 * 只查询一行的简易查询
 * 
 * 参数1: 查询的字段
 * 参数2: 表名
 * 参数3: 查询条件
 * 参数4(可选): 数据库连接相关的用户名密码库名等的更改
 * 
 * 返回值: 数组(一行结果mysqli_fetch_array处理后的数组)

function the_mysql_sel_one($ziduan, $table, $tiaojian, $arr = array())
{
    $str = "SELECT {$ziduan} FROM {$table} WHERE {$tiaojian} LIMIT 1";
    $res = the_mysql_go($str ,$arr);
    $res = mysqli_fetch_array($res);
    return $res;
}
 */


/**
 * 数字转曜日
 * 
 * 参数1: 0-6 (0->星期日 6->星期六) 如:date('w')
 * 
 * 返回值: 某曜日(字符串)
 * 
 * 2016-02-22
 */
function yaori($week_num)
{
    switch($week_num)
    {
        case 0: 
            return '日曜日';
            break;
        
        case 1: 
            return '月曜日';
            break;
        case 2: 
            return '火曜日';
            break;
        case 3: 
            return '水曜日';
            break;
        case 4: 
            return '木曜日';
            break;
        case 5: 
            return '金曜日';
            break;
        case 6: 
            return '土曜日';
            break;
        default:
            return '异时空曜日';
    }
}


/**
 * 数字转季节
 * 
 * 参数1: 1-12 (最好前面没导零) 如:输入date('n')
 * 
 * 返回值: 季节信息(字符串)
 * 
 * 2016-02-22
 */
function season($month)
{
    switch($month)
    {
        case 12:
        case 1:
        case 2: 
            return '冬';
            break;
        case 3:
        case 4:
        case 5: 
            return '春';
            break;
        case 6:
        case 7:
        case 8: 
            return '夏';
            break;
        case 9:
        case 10:
        case 11: 
            return '秋';
            break;
        default:
            return '异时空季节';
    }
}


/**
 * 根据ip获取天气
 * 用上了showapi.com的天气接口
 * 
 * 参数1: ip地址
 * 
 * 返回值: 天气信息(多维数组)
 * 
 * 2016-02-22
 */
function weather($ip)//(返回数组)
{
    $the_url = 'http://route.showapi.com/9-4?showapi_appid=15850&showapi_sign=05a656c81fb54aa2b01e6bba6ea232b6&showapi_timestamp=' . date('YmdHis') . '&ip=' . $ip . '&needAlarm=1';
    
    $con = curl_init();//句柄
    curl_setopt($con, CURLOPT_URL, $the_url);//地址
    curl_setopt($con, CURLOPT_TIMEOUT, 5);//超时
    curl_setopt($con, CURLOPT_RETURNTRANSFER, 1);//不直接输出
    //curl_setopt($con, CURLOPT_POST, 1);//post类型
    //curl_setopt($con, CURLOPT_POSTFIELDS, $the_post);//post数据
    $re = curl_exec($con);//一库
    curl_close($con);//关闭
    
    $info = json_decode($re, TRUE);
    
    //*有时不造为何抽风就没了些数据//**觉得这么写好笨...
    isset($info['showapi_res_body']['cityInfo']['c7']) or $info['showapi_res_body']['cityInfo']['c7'] = '暂无';
    isset($info['showapi_res_body']['cityInfo']['c5']) or $info['showapi_res_body']['cityInfo']['c5'] = '暂无';
    isset($info['showapi_res_body']['now']['temperature']) or $info['showapi_res_body']['now']['temperature'] = '暂无';
    isset($info['showapi_res_body']['f1']['day_air_temperature']) or $info['showapi_res_body']['f1']['day_air_temperature'] = '暂无';
    isset($info['showapi_res_body']['f1']['night_air_temperature']) or $info['showapi_res_body']['f1']['night_air_temperature'] = '暂无';
    isset($info['showapi_res_body']['now']['sd']) or $info['showapi_res_body']['now']['sd'] = '暂无';
    isset($info['showapi_res_body']['now']['wind_power']) or $info['showapi_res_body']['now']['wind_power'] = '暂无';
    isset($info['showapi_res_body']['f1']['jiangshui']) or $info['showapi_res_body']['f1']['jiangshui'] = '暂无';
    isset($info['showapi_res_body']['now']['weather']) or $info['showapi_res_body']['now']['weather'] = '暂无';
    isset($info['showapi_res_body']['f1']['day_weather']) or $info['showapi_res_body']['f1']['day_weather'] = '暂无';
    isset($info['showapi_res_body']['f1']['night_weather']) or $info['showapi_res_body']['f1']['night_weather'] = '暂无';
    return $info;
    //return var_dump($the_url);
}


/**
 * 获取客户端操作系统信息包括win10
 * @param  null
 * @author  Jea杨
 * @return string 
 */
function getBrowse()
{
     $sys = $_SERVER['HTTP_USER_AGENT'];  //获取用户代理字符串
     if (stripos($sys, "Firefox/") > 0) {
         preg_match("/Firefox\/([^;)]+)+/i", $sys, $b);
         $exp[0] = "Firefox";
         $exp[1] = $b[1];  //获取火狐浏览器的版本号
     } elseif (stripos($sys, "Maxthon") > 0) {
         preg_match("/Maxthon\/([\d\.]+)/", $sys, $aoyou);
         $exp[0] = "傲游";
         $exp[1] = $aoyou[1];
     } elseif (stripos($sys, "MSIE") > 0) {
         preg_match("/MSIE\s+([^;)]+)+/i", $sys, $ie);
         $exp[0] = "IE";
         $exp[1] = $ie[1];  //获取IE的版本号
     } elseif (stripos($sys, "OPR") > 0) {
             preg_match("/OPR\/([\d\.]+)/", $sys, $opera);
         $exp[0] = "Opera";
         $exp[1] = $opera[1];  
     } elseif(stripos($sys, "Edge") > 0) {
         //win10 Edge浏览器 添加了chrome内核标记 在判断Chrome之前匹配
         preg_match("/Edge\/([\d\.]+)/", $sys, $Edge);
         $exp[0] = "Edge";
         $exp[1] = $Edge[1];
     } elseif (stripos($sys, "Chrome") > 0) {
             preg_match("/Chrome\/([\d\.]+)/", $sys, $google);
         $exp[0] = "Chrome";
         $exp[1] = $google[1];  //获取google chrome的版本号
     } elseif(stripos($sys,'rv:')>0 && stripos($sys,'Gecko')>0){
         preg_match("/rv:([\d\.]+)/", $sys, $IE);
             $exp[0] = "IE";
         $exp[1] = $IE[1];
     }else {
        $exp[0] = "未知浏览器";
        $exp[1] = ""; 
     }
     return $exp[0].'('.$exp[1].')';
}



/**
 * 获取客户端操作系统信息包括win10
 * @param  null
 * @author  Jea杨
 * @return string 
 */
function getOS()
{
$agent = $_SERVER['HTTP_USER_AGENT'];
    $os = false;
 
    if (preg_match('/win/i', $agent) && strpos($agent, '95'))
    {
        $os = 'Windows 95';
    }
    else if (preg_match('/win 9x/i', $agent) && strpos($agent, '4.90'))
    {
        $os = 'Windows ME';
    }
    else if (preg_match('/win/i', $agent) && preg_match('/98/i', $agent))
    {
        $os = 'Windows 98';
    }
    else if (preg_match('/win/i', $agent) && preg_match('/nt 6.0/i', $agent))
    {
        $os = 'Windows Vista';
    }
    else if (preg_match('/win/i', $agent) && preg_match('/nt 6.1/i', $agent))
    {
        $os = 'Windows 7';
    }
      else if (preg_match('/win/i', $agent) && preg_match('/nt 6.2/i', $agent))
    {
        $os = 'Windows 8';
    }else if(preg_match('/win/i', $agent) && preg_match('/nt 10.0/i', $agent))
    {
        $os = 'Windows 10';#添加win10判断
    }else if (preg_match('/win/i', $agent) && preg_match('/nt 5.1/i', $agent))
    {
        $os = 'Windows XP';
    }
    else if (preg_match('/win/i', $agent) && preg_match('/nt 5/i', $agent))
    {
        $os = 'Windows 2000';
    }
    else if (preg_match('/win/i', $agent) && preg_match('/nt/i', $agent))
    {
        $os = 'Windows NT';
    }
    else if (preg_match('/win/i', $agent) && preg_match('/32/i', $agent))
    {
        $os = 'Windows 32';
    }
    else if (preg_match('/linux/i', $agent))
    {
        $os = 'Linux';
    }
    else if (preg_match('/unix/i', $agent))
    {
        $os = 'Unix';
    }
    else if (preg_match('/sun/i', $agent) && preg_match('/os/i', $agent))
    {
        $os = 'SunOS';
    }
    else if (preg_match('/ibm/i', $agent) && preg_match('/os/i', $agent))
    {
        $os = 'IBM OS/2';
    }
    else if (preg_match('/Mac/i', $agent) && preg_match('/PC/i', $agent))
    {
        $os = 'Macintosh';
    }
    else if (preg_match('/PowerPC/i', $agent))
    {
        $os = 'PowerPC';
    }
    else if (preg_match('/AIX/i', $agent))
    {
        $os = 'AIX';
    }
    else if (preg_match('/HPUX/i', $agent))
    {
        $os = 'HPUX';
    }
    else if (preg_match('/NetBSD/i', $agent))
    {
        $os = 'NetBSD';
    }
    else if (preg_match('/BSD/i', $agent))
    {
        $os = 'BSD';
    }
    else if (preg_match('/OSF1/i', $agent))
    {
        $os = 'OSF1';
    }
    else if (preg_match('/IRIX/i', $agent))
    {
        $os = 'IRIX';
    }
    else if (preg_match('/FreeBSD/i', $agent))
    {
        $os = 'FreeBSD';
    }
    else if (preg_match('/teleport/i', $agent))
    {
        $os = 'teleport';
    }
    else if (preg_match('/flashget/i', $agent))
    {
        $os = 'flashget';
    }
    else if (preg_match('/webzip/i', $agent))
    {
        $os = 'webzip';
    }
    else if (preg_match('/offline/i', $agent))
    {
        $os = 'offline';
    }
    else
    {
      $os = '未知操作系统';
    }
    return $os;  
}


/**
 * 获取访问者语言
 * 
 * 参数: 无
 * 
 * 返回值: 语言信息(字符串) / $_SERVER["HTTP_ACCEPT_LANGUAGE"]
 * 
 * 2016-02-22
 */
function getLanguage()
{
    $lang = substr($_SERVER['HTTP_ACCEPT_LANGUAGE'], 0, 4); //只取前4位，这样只判断最优先的语言。如果取前5位，可能出现en,zh的情况，影响判断。 
    if (preg_match("/zh-c/i", $lang)) {return '简体中文';} 
    else if (preg_match("/zh/i", $lang)) {return '繁体中文';}
    else if (preg_match("/en/i", $lang)) {return 'English';}
    else if (preg_match("/fr/i", $lang)) {return 'French';}
    else if (preg_match("/de/i", $lang)) {return 'German';}
    else if (preg_match("/jp/i", $lang)) {return 'Japanese';}
    else if (preg_match("/ko/i", $lang)) {return 'Korean';}
    else if (preg_match("/es/i", $lang)) {return 'Spanish';}
    else if (preg_match("/sv/i", $lang)) {return 'Swedish';}
    else return $_SERVER["HTTP_ACCEPT_LANGUAGE"];
}



/**
 * 通过ip查询所属地
 * 用到了91查的API
 * 
 * 参数1: ip地址
 * 
 * 返回值: 地区信息(字符串)
 * 
 * 2016-02-22
 */
function getLocal($ip)
{
    $url = 'http://api.91cha.com/ip?key=7043ca58366c4388a3333499d46ed325&ip='.$ip;
    $all = file_get_contents($url);
    $ccc = json_decode($all,TRUE);
    //$all = iconv("GB2312","utf8",$allz);
    $out = $ccc['data']['country'].'|'.$ccc['data']['area'].'|'.$ccc['data']['province'].'|'.$ccc['data']['city'].'|'.$ccc['data']['district'].'|'.$ccc['data']['linetype'];
    return $out;
}


/**
 * 获取访问者ip(貌似不受网页代理影响)
 * 
 * 参数: 无
 * 
 * 返回值: ip地址(字符串)
 * 
 * 2016-02-22
 */
function get_user_ip()
{
    if(getenv('HTTP_CLIENT_IP') && strcasecmp(getenv('HTTP_CLIENT_IP'), 'unknown'))
    {
        $ip = getenv('HTTP_CLIENT_IP');
    }
    elseif(getenv('HTTP_X_FORWARDED_FOR') && strcasecmp(getenv('HTTP_X_FORWARDED_FOR'), 'unknown'))
    {
        $ip = getenv('HTTP_X_FORWARDED_FOR');
    }
    elseif(getenv('REMOTE_ADDR') && strcasecmp(getenv('REMOTE_ADDR'), 'unknown'))
    {
        $ip = getenv('REMOTE_ADDR');
    }
    elseif(isset($_SERVER['REMOTE_ADDR']) && $_SERVER['REMOTE_ADDR'] && strcasecmp($_SERVER['REMOTE_ADDR'], 'unknown'))
    {
        $ip = $_SERVER['REMOTE_ADDR'];
    }
    return preg_match("/[\d\.]{7,15}/", $ip, $matches) ? $matches[0] : 'unknown';
}


/**
 * 转义html元素并直接输出 (如 < 成为 &lt;)
 * 
 * 参数: 要转义的字符串
 * 
 * 返回值: 无
 * 
 * 2016-02-22
 */
function echohtml($input_daima)
{
    echo htmlspecialchars($input_daima,ENT_QUOTES,"UTF-8");
}




/**
 * 转义html元素 (如 < 成为 &lt;)
 * 
 * 参数: 要转义的字符串
 * 
 * 返回值: 转义后的结果(字符串)
 * 
 * 2016-02-22
 */
function varhtml($input_daima) //
{
    return htmlspecialchars($input_daima,ENT_QUOTES,"UTF-8");
}




/**
 * 特殊符号前面加斜杠转义
 * 
 * 参数: 要转义的字符串
 * 
 * 返回值: 转义后的结果(字符串)
 * 
 * 2016-02-22
 */
function tophp($value)
{

    if (!get_magic_quotes_gpc())
    {
        $value = addslashes($value);
    }
/* 如果不是数字则加引号
    
    if (!is_numeric($value))
    {
        $value = "'" . mysql_real_escape_string($value) . "'";
    }
*/  
    return $value;
}

?>