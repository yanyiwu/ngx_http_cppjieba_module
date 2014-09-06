# `ngx_http_cppjieba_module`

## 简介

[CppJieba] 的 `Nginx` 扩展模块.

## 用法

### 安装和配置

以下用法假设 `ngx_http_cppjieba_module` 下载后存放的地址是 `/tmp/ngx_http_cppjieba_module` (这个地址在 `Nginx` 编译时和词典加载时候会用到)   

#### 下载源码：

```
git clone git://github.com/aszxqw/ngx_http_cppjieba_module.git /tmp/ngx_http_cppjieba_module
```

#### 进入 `Nginx` 源码目录：

```
./configure --add-module=/tmp/ngx_http_cppjieba_module/src
```

因为 `ngx_http_cppjieba_module` 是 `C++` 源码，所以作为 `Nginx` 模块编译的时候需要 修改 `obj/Makefile`

```
# 1. 在 "CC = gcc" 下面增加一行，如下
CXX = g++
# 2. 修改链接器为 g++ ， 如下
LINK = $(CXX)
# 3. 修改 ngx_http_cppjieba_module.cpp 的编译器，从 $(CC) 改为 $(CXX) ， 如下
$(CXX) -c $(CFLAGS)  $(ALL_INCS) \
    -o objs/addon/src/ngx_http_cppjieba_module.o \
    /tmp/ngx_http_cppjieba_module/src/ngx_http_cppjieba_module.cpp
# 4. 修改词典路径： /tmp/ngx_http_cppjieba_module/src/dict_path.h ， 如下
const char* const DICT_PATH = "/tmp/ngx_http_cppjieba_module/dict/jieba.dict.utf8";
const char* const HMM_PATH = "/tmp/ngx_http_cppjieba_module/dict/hmm_model.utf8";
const char* const USER_DICT_PATH = "/tmp/ngx_http_cppjieba_module/dict/user.dict.utf8";
```

```
# 5. 修改 Nginx 配置文件 /usr/local/nginx/conf/nginx.conf
location /cppjieba {
    cppjieba /tmp/ngx_http_cppjieba_module/dict/;
}
```

```
如果 `ngx_http_cppjieba_module` 的源码路径不是 `/tmp/ngx_http_cppjieba_module`， 将上述过程的 `/tmp/xxx` 改为 `/your/path/xxx` 即可。

## 启动 Nginx

```
/usr/local/nginx/sbin/nginx
```

## 测试 

```
curl "http://127.0.0.1/cppjieba?s=长春市长春药店"
```

预期结果：

```
["长春市", "长春", "药店"]
```

用 chrome 打开上述链接也可以，不过要记得设置浏览器的页面编码方式为 `utf-8` 。

## 客服

`i@yanyiwu.com`

[CppJieba]:https://github.com/aszxqw/cppjieba
