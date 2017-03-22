<?php
/**
 * Cache Service.
 * User: gukai@bilibili.com
 * Date: 16/12/13
 * Time: 下午12:46
 */
class Service
{

    /**
     * @var array
     * NoSQL类型
     */
    protected $nosql = ['Memcached', 'Redis', 'MongoDB'];

    /**
     * @var null
     * 适配器对象
     */
    protected $adapter;

    /**
     * @var Memcached|Redis
     * 执行器对象
     */
    protected $executor;


    /**
     * Service constructor.
     * @param null $adapter
     */
    public function __construct($adapter = null)
    {
        if (!in_array($adapter, $this->nosql)) {
            $this->adapter = 'Memcached';
        } else {
            $this->adapter = $adapter;
        }

        switch ($this->adapter) {
            case 'Memcached':
                $exec = new Memcached();
                $exec->addServer('127.0.0.1', '11211');
            break;
            case 'Redis':
                $exec = new Redis();
                $exec->connect('127.0.0.1', '6379');
            break;
            case 'MongoDB':
                // $exec = new MongoDB();
            break;
        }

        $this->executor = $exec;
    }


    /**
     * @param $name
     * @return bool|mixed|null|string
     */
    public function get($name)
    {
        if (!$result = $this->executor->get($name)) {
            return false;
        }

        return $result;
    }


    /**
     * @param $name
     * @param $obj
     * @param int $expires
     * @return bool|null
     */
    public function set($name, $obj, $expires = -1)
    {
        if (!$name) {
            return false;
        }

        return $this->executor->set($name, $obj, $expires);
    }


    /**
     * @param $name
     * @param $obj
     * @param int $expires
     * @return bool|null
     */
    public function replace($name, $obj, $expires = -1)
    {
        return $this->set($name, $obj, $expires);
    }


    /**
     * @param $name
     * @return bool|null|void
     */
    public function delete($name)
    {
        if (!$name) {
            return false;
        }

        return $this->executor->delete($name);
    }


    /**
     * @param null $selectDB
     * @return bool
     */
    public function flush($selectDB = null)
    {
        if ('memcached' === strtolower($this->adapter)) {
            $this->executor->flush();
        } else {
            $this->executor->flushDB($selectDB);
        }

        return true;
    }

}