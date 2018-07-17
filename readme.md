# C语言版多线程文件拷贝

> 在 Linux系统下生成可执行文件copy

```Cpp
gcc pthread_copy.cpp -o copy
```

> 利用copy执行文件复制文件

```Cpp
//复制文件夹及文件夹内的所有文件
copy srcdir/  tagdir/

//复制单个文件 ,如果文件存在则覆盖，不存在则创建
copy srcdir/test.txt  tagdir/
```
