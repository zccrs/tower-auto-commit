<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE TS>
<TS version="2.1" language="zh_CN">
<context>
    <name>QObject</name>
    <message>
        <location filename="../main.cpp" line="28"/>
        <source>not found %1 record.</source>
        <translation>未找到 %1 记录</translation>
    </message>
    <message>
        <location filename="../main.cpp" line="65"/>
        <source>email address.</source>
        <translation>邮箱地址</translation>
    </message>
    <message>
        <location filename="../main.cpp" line="66"/>
        <source>password.</source>
        <translation>密码</translation>
    </message>
    <message>
        <location filename="../main.cpp" line="67"/>
        <source>date(format=yyyy-M-d), default is system current date.</source>
        <translation>日期（格式=yyyy-M-d），默认是系统当前时间</translation>
    </message>
    <message>
        <location filename="../main.cpp" line="70"/>
        <source>input weekly mode([input|file|command], we will use dialogue mode is not set).</source>
        <translation>输入周报的模式（[input|file|command]，未指定时将使用对话模式询问所有周报必填项）</translation>
    </message>
    <message>
        <location filename="../main.cpp" line="72"/>
        <source>if you specify the email then set as default user, else use the default user as a email value.</source>
        <translation>如果指定了email则设置此email为默认，否则使用当前记录的默认email登录</translation>
    </message>
    <message>
        <location filename="../main.cpp" line="73"/>
        <source>clear info([user|default|record|all]).
--clear=user: clear user info.
--clear=default: clear default email.
--clear=all: clear all config(contain record).
--clear=record: clear record, if not set record then clear all record.</source>
        <translation>清除信息（[user|default|record|all]）
--clear=user：清除当前用户的信息（根据email的值）
--clear=default：清除默认的email
--clear=all：清空配置文件（record的记录也会清除）
--clear=record：清除指定的命令记录，如果未指定--record，则清除全部的命令记录</translation>
    </message>
    <message>
        <source>clear info([user|default|record|all]).
--clear=user: clear user info.
--clear=default: clear default email.
--clear=record: clear record, if not set record then clear all record.</source>
        <translation type="vanished">清除信息（[user|default|record|all]）
--clear=user：清除当前用户的信息（根据email的值）
--clear=default：清除默认的email
--clear=record：清除指定的命令记录，如果未指定--record，则清除全部的命令记录</translation>
    </message>
    <message>
        <location filename="../main.cpp" line="78"/>
        <source>record this command and assign a name, if the name exists  then replace command.</source>
        <translation>记录当前输入的命令。记录过的命令可以直接根据记录名称来执行，例如：“tower-tool --record a -w”，则执行“tower-tool --exec a”等同于执行“tower-tool -w”</translation>
    </message>
    <message>
        <location filename="../main.cpp" line="79"/>
        <source>execute recorded command.</source>
        <translation>执行已记录的命令</translation>
    </message>
    <message>
        <location filename="../main.cpp" line="92"/>
        <source>--mode=input: weekly data(format=JSON).
--mode=file: file path(text-encoding=UTF-8, format=JSON).
--mode=command: command and arguments(text-encoding=UTF-8, format=JSON).</source>
        <translation>
--mode=input：直接输入周报内容（格式=JSON）
--mode=file：输入文件路径，将文件内容作为周报的内容（编码=UTF-8，格式=JSON）
--mode=command：输入要执行的命令，将程序返回结果作为周报内容（格式=JSON）</translation>
    </message>
    <message>
        <location filename="../main.cpp" line="92"/>
        <source>weekly data</source>
        <translation>周报数据</translation>
    </message>
    <message>
        <location filename="../main.cpp" line="129"/>
        <source>email unspecified.</source>
        <translation>没有指定email</translation>
    </message>
    <message>
        <location filename="../main.cpp" line="167"/>
        <source>Will be &quot;%1&quot; replacement for the &quot;%2&quot;.</source>
        <translation>将使用“%1”替换掉“%2”</translation>
    </message>
    <message>
        <source>Will be %1 replacement for the %2.</source>
        <translation type="vanished">将使用“%1”替换掉“%2”</translation>
    </message>
    <message>
        <location filename="../main.cpp" line="215"/>
        <source>weekly content is empty.</source>
        <translation>获取到的周报内容是空的</translation>
    </message>
    <message>
        <source>arguments is empty.</source>
        <translation type="vanished">参数是空的</translation>
    </message>
    <message>
        <source>date, format=</source>
        <translation type="vanished">日期，格式=</translation>
    </message>
    <message>
        <source>, default is system current date.</source>
        <translation type="vanished">，默认是系统的当前时间</translation>
    </message>
    <message>
        <location filename="../main.cpp" line="68"/>
        <source>get week index by date.</source>
        <translation>获取指定日期是哪年的第几周</translation>
    </message>
    <message>
        <source>input weekly mode([input|file|command], default=input).</source>
        <translation type="vanished">获取周报内容的模式（[input|file|command]，默认=input）</translation>
    </message>
    <message>
        <location filename="../main.cpp" line="71"/>
        <source>save user info to local.</source>
        <translation>保存用户信息到本地</translation>
    </message>
    <message>
        <source>set as default user.</source>
        <translation type="vanished">设置为默认用户</translation>
    </message>
    <message>
        <source>clear info([user|default]).</source>
        <translation type="vanished">清除信息([user|default])</translation>
    </message>
    <message>
        <location filename="../main.cpp" line="69"/>
        <source>according keyword filter weekly item(keyword format is regular expressions, default is 必填).</source>
        <translation>根据关键字过滤周报条目（关键字格式是正则表达式，默认关键字是“必填”）</translation>
    </message>
    <message>
        <source>is weekly data.</source>
        <translation type="vanished">，直接输入周报内容（格式=JSON）</translation>
    </message>
    <message>
        <source>
mode == </source>
        <translation type="vanished">
模式 == </translation>
    </message>
    <message>
        <source>is file path.
mode == </source>
        <translation type="vanished">，传入文件的绝对路径
模式 == </translation>
    </message>
    <message>
        <source>is command and arguments.</source>
        <translation type="vanished">，传入需要执行的命令及其参数</translation>
    </message>
    <message>
        <source>setting get weekly modes</source>
        <translation type="vanished">设置输入周报的模式</translation>
    </message>
    <message>
        <source>], default=</source>
        <translation type="vanished">]，默认=</translation>
    </message>
    <message>
        <source>
input: key in data (format=JSON).
file: the contents of the file as weekly data (text-encoding=UTF-8, format=JSON).
command: exec command, it returns the contents as weekly data (text-encoding=UTF-8, format=JSON).</source>
        <translation type="vanished">
input：数据格式=JSON
file：将文件内容指定为周报数据（文本编码=UTF-8，数据格式=JSON）
command：将执行命令返回的数据作为周报内容（数据格式=JSON）</translation>
    </message>
    <message>
        <source>mode == </source>
        <translation type="vanished">模式 == </translation>
    </message>
</context>
<context>
    <name>Weekly</name>
    <message>
        <location filename="../weekly.cpp" line="94"/>
        <source>input email: </source>
        <translation>请输入email：</translation>
    </message>
    <message>
        <location filename="../weekly.cpp" line="104"/>
        <source>current email is:</source>
        <translation>当前email是：</translation>
    </message>
    <message>
        <location filename="../weekly.cpp" line="112"/>
        <source>Enter the password and saved locally inconsistent, whether to update the saved data? [n/Y]</source>
        <translation>输入的密码和保存在本地的密码不一致，是否要替换？[n/Y]</translation>
    </message>
    <message>
        <location filename="../weekly.cpp" line="125"/>
        <source>input password: </source>
        <translation>请输入密码：</translation>
    </message>
    <message>
        <location filename="../weekly.cpp" line="155"/>
        <source>Data is not json array.</source>
        <translation>数据格式不是JSON数组</translation>
    </message>
    <message>
        <location filename="../weekly.cpp" line="326"/>
        <source>invalid regular expression:</source>
        <translation>无效的正则表达式：</translation>
    </message>
    <message>
        <location filename="../weekly.cpp" line="333"/>
        <source>weekly match any item not found, regular expression pattern is:</source>
        <translation>未找到符合规则的周报项，正则表达式内容是：</translation>
    </message>
    <message>
        <location filename="../weekly.cpp" line="351"/>
        <source>(Press Ctrl+D to the next step): </source>
        <translation>按下Ctrl+D可进行下一项的输入</translation>
    </message>
    <message>
        <location filename="../weekly.cpp" line="381"/>
        <source>title:</source>
        <translation>标题：</translation>
    </message>
    <message>
        <location filename="../weekly.cpp" line="382"/>
        <source>content:</source>
        <translation>内容：</translation>
    </message>
    <message>
        <location filename="../weekly.cpp" line="386"/>
        <source>weekly content is empty.</source>
        <translation>输入的周报内容是空的</translation>
    </message>
    <message>
        <location filename="../weekly.cpp" line="409"/>
        <source>Success</source>
        <translation>完成</translation>
    </message>
    <message>
        <location filename="../weekly.cpp" line="417"/>
        <source>invalid regular regular expression match iterator:</source>
        <translation>正则表达式匹配的结果是无效的：</translation>
    </message>
    <message>
        <location filename="../weekly.cpp" line="499"/>
        <source>request url:</source>
        <translation>目标url：</translation>
    </message>
    <message>
        <source>request data:</source>
        <translation type="vanished">附带的数据：</translation>
    </message>
</context>
</TS>
