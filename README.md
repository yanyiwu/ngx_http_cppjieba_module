# `cppjieba_nginx_module`

## 简介

[cppjieba] 的 nginx 扩展模块.

## 用法

下载源码：

```
git clone git://github.com/aszxqw/cppjieba_nginx_module.git /tmp/cppjieba_nginx_module
```


```
./configure --add-module=/home/yanyiwu/code/cppjieba_nginx_module/src
```

修改 obj/Makefile

```
# 1. add one line bellow "CC = gcc"
CXX = g++
# 2. change the way for linking.
LINK = $(CXX)
# 3. change the way for compiling cppjieba_module.cpp
$(CXX) -c $(CFLAGS)  $(ALL_INCS) \
    -o objs/addon/src/cppjieba_nginx_module.o \
    /tmp/cppjieba_nginx_module/src/cppjieba_nginx_module.cpp
# 4. modify /usr/local/nginx/conf/nginx.conf
location /cppjieba {
    cppjieba /tmp/cppjieba_nginx_module/dict/;
}
```

## 客服

`i@yanyiwu.com`

[cppjieba]:https://github.com/aszxqw/cppjieba
