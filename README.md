# FtpServer
This is a FtpServer for Linux(Windows not support now).

使用方法：

直接执行./build.sh就可以进行编译，build.sh会调用根目录下的makefile对相关.cpp进行编译，
之后会生成ftpserver的可执行文件，直接./ftpserver就可以运行。（使用前先配置lua脚本，lua脚本写入的是你要实现的根目录，即是客户端能看到的根目录）

使用前，先配置config.lua的文件根目录（即是FTP服务器所显示的根目录）
