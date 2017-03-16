<?php

$buf = new bufItem('hello', 'world', 2);
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
