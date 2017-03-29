<?php
/**
 * Buffer Item.
 * User: gukai@bilibili.com
 * Date: 16/12/12
 * Time: 下午4:44
 */
class CacheItem
{
    /**
     * @var string
     * 当前对象所映射的键值
     */
    public $key;

    /**
     * @var mixed
     * 链表上一个节点
     */
    public $prev;

    /**
     * @var mixed
     * 链表下一个节点
     */
    public $next;

    /**
     * @var int
     * 对象创建时间
     */
    protected $createTime;

    /**
     * @var int
     * 对象过期时间
     */
    protected $expireTime;

    /**
     * @var mixed
     * 对象实体元素
     */
    protected $entity;

    /**
     * @var int
     * 调用次数方便做频率计算
     */
    protected $hitCount;


    /**
     * CacheItem constructor.
     * @param string $key
     * @param mixed $value
     * @param int $expires
     */
    public function __construct($key, $value, $expires = -1)
    {
        $this->key = $key;
        $this->createTime = microtime(true);
        $this->expireTime = $expires;
        $this->entity = $value;
        $this->hitCount = 1;
    }


    /**
     * @param  void
     * @return bool
     */
    public function isExpired()
    {
        return ($this->expireTime != -1 && microtime(true) - $this->createTime > $this->expireTime);
    }


    /**
     * @param  mixed $value
     * @return mixed
     */
    public function setPrev($value)
    {
        return $this->prev = $value;
    }


    /**
     * @param  void
     * @return mixed
     */
    public function getPrev()
    {
        return $this->prev;
    }


    /**
     * @param  mixed $value
     * @return mixed
     */
    public function setNext($value)
    {
        return $this->next = $value;
    }


    /**
     * @param  void
     * @return mixed
     */
    public function getNext()
    {
        return $this->next;
    }


    /**
     * @param  mixed $value
     * @return mixed
     */
    public function setEntity($value)
    {
        $this->hitCount++;
        return $this->entity = $value;
    }


    /**
     * @param  void
     * @return mixed
     */
    public function getEntity()
    {
        $this->hitCount++;
        return $this->entity;
    }


    /**
     * @param  void
     * @return string
     */
    public function getKey()
    {
        return $this->key;
    }


    /**
     * @param  void
     * @return int
     */
    public function getHitCount()
    {
        return $this->hitCount;
    }


    /**
     * @param  void
     * @return int|mixed
     */
    public function getTime()
    {
        return $this->createTime;
    }


    /**
     * @param  void
     * @return mixed|null
     */
    public function __invoke()
    {
        return $this->isExpired() ? null : $this->getEntity();
    }

}

