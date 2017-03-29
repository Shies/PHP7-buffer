<?php
/**
 * Buffer/Connect/Thread Pool
 * User: gukai@bilibili.com
 * Date: 16/12/12
 * Time: 下午8:03
 */
class CachePool
{
    /**
     * @var CacheItem
     * 链表头部
     */
    protected $head;

    /**
     * @var CacheItem
     * 链表尾部
     */
    protected $tail;

    /**
     * @var array
     * 采用lru+hashmap实时过滤
     */
    protected $hashmap;

    /**
     * @var int
     * 容器大小
     */
    protected $capacity;


    /**
     * CachePool constructor.
     * @param $capacity
     */
    public function __construct($capacity)
    {
        $this->init($capacity);
    }


    /**
     * @param $capacity
     */
    private function init($capacity)
    {
        $this->hashmap = [];
        $this->capacity = $capacity;

        $this->head = new CacheItem(null, null);
        $this->tail = new CacheItem(null, null);
        $this->head->setNext($this->tail);
        $this->tail->setPrev($this->head);

        return !false;
    }



    /**
     * @param  void
     * @return $this
     */
    public function clear()
    {
        $this->hashmap = [];
        $this->head = $this->tail = null;

        return $this;
    }



    /**
     * @param  null|string $key
     * @return null
     */
    public function get($key)
    {
        // 检查当前节点对象是否存活
        if (!isset($this->hashmap[$key])) {
            return false;
        }

        $node = $this->hashmap[$key];
        if (sizeof($this->hashmap) > 0) {
            // refresh the access
            $this->detach($node)->attach($this->head, $node);
        }

        return $node->getEntity();
    }



    /**
     * @param string  $key
     * @param object  $value
     * @param integer $expires
     * @return bool
     */
    public function set($key, $value, $expires = -1)
    {
        if (0 >= $this->capacity) {
            return false;
        }

        if (isset($this->hashmap[$key])) {
            $node = $this->hashmap[$key];
            $node->setEntity($value);

            // update data
            $this->detach($node)->attach($this->head, $node);
            return true;
        }

        $node = new CacheItem($key, $value, $expires);
        $this->hashmap[$key] = $node;
        $this->attach($this->head, $node);


        return $this->release();
    }



    /**
     * @param  void
     * @return int
     */
    private function release()
    {
        // check if cache is full
        if ($this->capacity < sizeof($this->hashmap)) {
            // we're full, remove the tail
            $nodeToRemove = $this->tail->getPrev();
            $this->detach($nodeToRemove);
            unset($this->hashmap[$nodeToRemove->getKey()]);
        }

        return !false;
    }



    /**
     * @param $head
     * @param $node
     * @return $this
     */
    private function attach($head, $node)
    {
        $node->setPrev($head);
        $node->setNext($head->getNext());
        $node->getNext()->setPrev($node);
        $node->getPrev()->setNext($node);

        return $this;
    }



    /**
     * @param $node
     * @return $this
     */
    private function detach($node)
    {
        $node->getPrev()->setNext($node->getNext());
        $node->getNext()->setPrev($node->getPrev());

        return $this;
    }



    /**
     * @param  void
     * @return mixed
     */
    public function checkout()
    {
        $key = array_rand($this->hashmap);
        return strval($key);
    }



    /**
     * @param string $key
     * @return null
     */
    public function delete($key)
    {
        if (!isset($this->hashmap[$key])) {
            return false;
        }

        $obj = $this->detach($this->hashmap[$key]);
        if (null !== $obj) {
            unset($obj->hashmap[$key]);
        }

        return (true);
    }



    /**
     * @param  void
     * @return int
     */
    public function size()
    {
        return sizeof($this->hashmap);
    }



    /**
     * @param  void
     * @return array
     */
    public function proto()
    {
        return $this->hashmap;
    }



    /**
     * @param  void
     * @return bool|null
     */
    public function __destruct()
    {
        (new CacheMath($this))->gc();
    }


}