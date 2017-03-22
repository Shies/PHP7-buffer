<?php
/**
 * Buffer Math.
 * User: gukai@bilibili.com
 * Date: 16/12/15
 * Time: 下午6:06
 */
class CacheMath
{

    /**
     * @var int
     * 定期清理垃圾个数
     */
    const CONTAINER_SIZE = 10;


    /**
     * CacheMath constructor.
     * @param CachePool $pool
     */
    public function __construct(CachePool $pool)
    {
        $this->pool = $pool;
    }


    /**
     * @return bool
     */
    public function isFull()
    {
        return $this->pool->size() > static::CONTAINER_SIZE;
    }


    /**
     * 使用频率最小移除法
     *
     * @param  null
     * @return bool
     */
    public function lfu()
    {
        if ($this->isFull()) {
            return true;
        }

        // 这里仅供参考, 真正应用不应该全表遍历
        static $ret = [];
        if (empty($ret)) {
            $proto = $this->pool->proto();
            foreach ($proto AS $key => $val) {
                $ret[] = ['hit' => $val->getHitCount(), 'ctime' => $val->getTime()];
            }
        }

        array_multisort(array_column($ret, 'hit'), SORT_DESC, array_column($ret, 'ctime'), SORT_DESC, $ret);
        $pop = end($ret);
        $this->pool->delete($pop->getKey());
    }


    /**
     * 先进先出
     *
     * @param  void
     * @return bool
     */
    public function fifo()
    {
        if ($this->isFull()) {
            return true;
        }

        static $ret = [];
        if (empty($ret)) {
            $proto = $this->pool->proto();
            foreach ($proto AS $key => $val) {
                $ret[$key] = is_object($val) ? $val->createTime : microtime(true);
            }
        }

        asort($ret);
        reset($ret);
        $this->pool->delete(key($ret));
    }


    /**
     * 垃圾回收
     *
     * @param  void
     * @return bool
     */
    public function gc()
    {
        // O(n) 顺序查找直接T出过期对象
        foreach ($this->pool->proto() AS $key => $val) {
            if ($val instanceof CacheItem && $val->isExpired()) {
                $this->pool->delete($key);
                break;
            }
        }

        return true;
    }


    /**
     * 随机移除
     *
     * @param  void
     * @return bool|null
     */
    public function rand()
    {
        if ($this->isFull()) {
            return true;
        }

        if ($key = $this->pool->checkout()) {
            return $this->pool->delete($key);
        }

        return !false;
    }

}