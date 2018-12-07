Makefile_ctest.inc是支持Cunit的Makefile。


Cunit操作流程
1，先参考Cunit简单用例编写测试相关代码(test_hello.c)。
2，make ctest=1编译好后，执行测试用例，得到2个xml。
3，利用icov转换得到代码覆盖率的可视化结果。
lcov -c -d ./ -o app.info
再利用app.info生成统计结果
genhtml app.info -o cc_result

使用浏览器打开cc_result/index.html就可以了


注意事项：
test_*的文件都是用来编写测试用例的代码，普通文件不要取test_开头的命名名字，前缀可以通过变量CTEST_PRIFIX指定(test_%)

想要生成测试用例，执行make ctest=1或者在Makefile里面添加变量CTEST_EXEC=xxx，编译出来的正式程序最好不要使用，使用make重新编译(需要makefile里面没有CTEST_EXEC变量)。

