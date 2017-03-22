<?php

# $buf = new Item('hello', 'world', -2);
# var_dump(call_user_func($buf)) . PHP_EOL;
# echo $buf->getHitCount() . PHP_EOL;
# echo $buf->getTime() . PHP_EOL;
# echo $buf->getKey() . PHP_EOL;
# echo $buf->setEntity('foo') . PHP_EOL;
# echo $buf->getHitCount() . PHP_EOL;
# var_dump(call_user_func($buf)) . PHP_EOL;

/*
var_dump($buf->createTime) . PHP_EOL;
var_dump($buf->expireTime) . PHP_EOL;
var_dump($buf->key) . PHP_EOL;
var_dump($buf->entity) . PHP_EOL;
var_dump($buf->hitCount) . PHP_EOL;
var_dump($buf->isExpired()) . PHP_EOL;
// ---------------------------- //
var_dump($buf->setPrev('foo'));
var_dump($buf->getPrev());
// ---------------------------- //
var_dump($buf->setNext('bar'));
var_dump($buf->getNext());
*/

$p = new Pool(10);
var_dump($p->capacity);
var_dump($p->head);
var_dump($p->tail);
var_dump($p->getNext());
var_dump($p->getPrev());
