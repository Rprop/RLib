<?php
/**
 * 本页面载入的前置是 ini.php
 */




/**
 * 数据库连接
 * 
 * 参数(可选)(数组): 
 * $host 数据库地址
 * $username 用户名
 * $passwd 密码
 * $dbname 数据库名
 * 
 * 返回值: 打开的连接(mysql资源)
 * 
 * 2016-02-23
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
 * 参数3(可选)(数组): ↓
 * 是否查询后手动关闭连接close
 * 调试开关debug 
 * 记录开关note(既查询成功并记录查询语句)
 * 错误后继续脚本continue
 * 默认都为FALSE
 * 
 * 要先转义关键值再传值
 * 
 * 返回值: 生结果集(结果集对象)
 * 
 * 2016-02-23
 */
function the_mysql_query($con, $str, $arr = array())
{
    $debug = FALSE;
    $note = FALSE;
    $close = FALSE;
    $continue = FALSE;
    extract($arr);//拆开数组(覆盖)

    //$str = mysqli_real_escape_string($con, $str);//转义
    if($debug){echo "[$str]";}//调试用, 直接输出查询的语句
    $res = mysqli_query($con, $str) or the_mysql_error(array('message' => "数据库查询语句执行出错| 查询语句: {$str}| 错误信息: " . mysqli_error($con), 'continue' => $continue));
    
    if($close){mysqli_close($con);}
    
    if($note){the_mysql_error(array('message' => $str, 'note' => TRUE));}//记录用
    
    return $res;
}


/**
 * 查询语句执行错误记录
 * (日志默认回车符为Unix的)
 * 
 * 参数(可选)(数组): 错误信息message(字符串) 记录note(布尔值)(默认为假, 真则不中断脚本)
 * 
 * 2016-02-23
 */
function the_mysql_error($arr = array())
{
    $message = '木有错误信息';
    $note = FALSE;
    $continue = FALSE;
    extract($arr);
    
    $str = "[ERROR]\n";
    if($note){$str = "[NOTE]\n";}

    $str .= date('Y-m-d H:i:s') . '  ip: ' . get_user_ip() . "\n" . $message . "\n\n";

    $handle = fopen(INI_PATH_SQL_ERROR, 'a+');
    fwrite($handle, $str);
    fclose($handle);
    
    if($note == FALSE && $continue == FALSE){die('数据库错误');}
    
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
 * 数字转星期
 * 
 * 参数1: 0-6 (0->星期日 6->星期六) 如:date('w')
 * 参数2(可选)(数组): 返回值种类type 默认为0
 * 可自行修改函
 * 返回值: 星期几(字符串)
 * 
 * 2016-02-23
 */
function the_week($week_num, $arr = array())
{
    $type = 0;
    $week_word_str[0] = '星期日|星期一|星期二|星期三|星期四|星期五|星期六';//以"|"分割
    $week_word_str[1] = '日曜日|月曜日|火曜日|水曜日|木曜日|金曜日|土曜日';
    extract($arr);
    
    $week_word_arr = explode('|', $week_word_str[$type]);

    $week = isset($week_word_arr[$week_num]) ? $week_word_arr[$week_num] : '异时空星期';
    
    return $week;
}


/**
 * 数字转季节
 * 
 * 参数1: 1-12 (最好前面没导零) 如:输入date('n')
 * 
 * 返回值: 季节信息(字符串)
 * 
 * 2016-02-23
 */
function the_season($month)
{
    $month--;
    $season_num = floor($month/3);
    $season_arr = explode('|', '冬|春|夏|秋');
    
    $season = isset($season_arr[$season_num]) ? $season_arr[$season_num] : '异时空季节';
    return $season;
}



/**
 * 通过ip查询所属地
 * 用到了91查的API
 * 
 * 参数1: ip地址
 * 
 * 返回值: 地区信息(字符串)
 * 
 * 2016-02-23
 */
function the_local($ip)
{
    $the_url = 'http://api.91cha.com/ip?key=7043ca58366c4388a3333499d46ed325&ip='.$ip;
    $con = curl_init();//句柄
    curl_setopt($con, CURLOPT_URL, $the_url);//地址
    curl_setopt($con, CURLOPT_TIMEOUT, 5);//超时
    curl_setopt($con, CURLOPT_RETURNTRANSFER, 1);//不直接输出
    $re = curl_exec($con);//一库
    curl_close($con);//关闭
    
    if($re)
    {
        $info = json_decode($re, TRUE);
        if($info['state'] != 1)
        {
            $info = FALSE;
        }
    }
    else
    {
        $info = FALSE;
    }
    
    
    return $info;
}



/**
 * 根据ip获取天气
 * 用上了showapi.com的天气接口
 * 
 * 参数1: ip地址
 * 
 * 返回值: 天气信息(多维数组)/获取失败则为FALSE
 * 
 * 2016-02-23
 */
function the_weather($ip)//(返回数组)
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
    
    if($re)
    {
        $info = json_decode($re, TRUE);
        if($info['showapi_res_code'] === 0)
        {
            isset($info['showapi_res_body']['f1']['jiangshui']) or $info['showapi_res_body']['f1']['jiangshui'] = '暂无';//暂时只知道这个有时会没有
        }
        else
        {
            $info = FALSE;
        }
    }
    else
    {
        $info = FALSE;
    }
    
    
    return $info;
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
 * 获取访问者ip(貌似不受网页代理影响)
 * 
 * 参数: 无
 * 
 * 返回值: ip地址(字符串)
 * 
 */
function getIP()
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


?>