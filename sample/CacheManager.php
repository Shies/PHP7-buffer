<?php
/**
 * Buffer Manager.
 * User: gukai@bilibili.com
 * Date: 16/12/15
 * Time: 下午5:22
 */
class CacheManager
{

    /**
     * @var object
     * 单例对象
     */
    protected static $instance;

    /**
     * @var array
     * 缓存池子
     */
    protected static $pool;

    /**
     * can implements interface
     * @var string|object
     */
    protected $adapter;


    /**
     * CacheManager constructor.
     */
    public function __construct()
    {
        static::$pool = [];
    }


    /**
     * @param  void
     * @return mixed
     */
    public static function getInstance()
    {
        if (null === static::$instance) {
            static::$instance = new static();
        }

        return static::$instance;
    }


    /**
     * @param string $name
     * @return bool
     */
    public function load($name)
    {
        if (is_null($name)) {
            return false;
        }

        if (empty(static::$pool[$name])) {
            static::$pool[$name] = new CachePool(CacheMath::CONTAINER_SIZE);
        }

        return static::$pool[$name];
    }


    /**
     * @param  void
     * @return mixed
     */
    public function getAdapter()
    {
        return $this->adapter;
    }


    /**
     * @param  void
     * @return array
     */
    public function getPool()
    {
        return static::$pool;
    }

}