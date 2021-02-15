# buffer
=======

## Introduction(介绍)

1. 采用C写的扩展完全不担心性能问题
2. 采用HashMap数据结构和LRU算法支持此扩展
3. 采用进程级共享本地缓存（local cache），能友好的预备热冷数据
4. 提供类似于php static静态池，伸缩方便
5. 参考Java实现dynamic cache的生命周期

## Feature(特点)

```text
能友好的分析热点和区分冷数据，并采用相应的措施，多进程怎么共享local cache呢？
我们可以借用swoole_process来实现进程通信，另外达到把单进程数据传送给其它进程，
大家都知道fpm进程数有限制，并发时，单台机器multi proc共享cache只能通过signal，
最终作用避免了网络io开销
```

## Design Origin(来源)

https://infinite.iteye.com/blog/126753

## Installation(安装PHP扩展)

1. git clone https://github.com/Shies/PHP7-buffer.git
2. cd PHP7-buffer
3. phpize
4. ./configure --with-php-config=/php-path/bin/php-config
5. make && make install
6. echo "extension=buffer.so" >> /php-path/etc/php.ini

## Usage(使用方法)

```php
<?php

$buf = new Item('hello', 'world', -2);
echo $buf->getHitCount() . PHP_EOL;
echo $buf->getTime() . PHP_EOL;
echo $buf->getKey() . PHP_EOL;
echo $buf->setEntity('foo') . PHP_EOL;
echo $buf->getHitCount() . PHP_EOL;
var_dump(call_user_func($buf)) . PHP_EOL;

$p = new Pool(10);
var_dump($p->set('foo', 'bar'));
var_dump($p->get('foo'));
var_dump($p->checkout());

$con = OrgManager::getInstance()->load('cache');
var_dump($con->set("foo", "bar"));
var_dump($con->get("foo"));

```

## Test Sample(测试样板)

```shell
➜  PHP7-buffer git:(master) ✗ ~/debug/php70-debug/bin/php -f buffer.php
sample/
├── CacheItem.php
├── CachePool.php
├── CacheManager.php
├── CacheMath.php
└── Service.php

0 directories, 5 files
```

