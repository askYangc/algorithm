Makefile文件有几个注意事项：
1，Makefile连接EXEC的时候调用的是gcc,如果有c++代码， 这里需要修改为g++，或者链接时加上-lstdc++。这里建议在Makefile里面加一个判断，暂时就不加了。
2, g++ -E的效果暂时还不知道。不考虑。