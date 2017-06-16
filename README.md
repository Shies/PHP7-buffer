# buffer
=======

## Introation

1. 采用C写的扩展完全不担心性能问题
2. 采用HashMap数据结构和LRU算法支持此扩展
3. 此缓冲池支持数据存进buffer中缓存(暂且不支持持久化)

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
├── CacheManager.php
├── CacheMath.php
├── CachePool.php
├── CacheMath.php
└── Service.php

0 directories, 6 files
```

