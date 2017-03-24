/*
  +----------------------------------------------------------------------+
  | PHP Version 7                                                        |
  +----------------------------------------------------------------------+
  | Copyright (c) 1997-2016 The PHP Group                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author: ___Shies <gukai@bilibili.com>                                |
  +----------------------------------------------------------------------+
*/

/* $Id$ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "main/SAPI.h"
#include "Zend/zend_API.h"
#include "Zend/zend_alloc.h"
#include "Zend/zend_interfaces.h"

#include "php_buffer.h"
#include "buffer_item.h"
#include "buffer_pool.h"
#include "ext/standard/info.h"
#include "ext/standard/php_string.h"
#include "ext/standard/php_var.h"


zend_class_entry *buffer_pool_ce;


PHP_METHOD(buffer_pool, __construct)
{
    int capacity, result;
    zval *self, *params;
    zval retval, method, map, _itemval;
    zend_bool slient;
    zval rv = {{0}};
    zval itemval = {{0}};


    self = getThis();
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &capacity) == FAILURE) {
        return;
    } else {
        array_init(&map);
        zend_update_property_long(buffer_pool_ce, self, ZEND_STRL("capacity"), capacity);
        zend_update_property(buffer_pool_ce, self, ZEND_STRL("hashmap"), &map);
    }

    // 属性显示不出来结构,所以new两次Item,区分不同的对象
    object_init_ex(&_itemval, buffer_item_ce);
    object_init_ex(&itemval, buffer_item_ce);

    zval knull, vnull;
    ZVAL_NULL(&knull);
    ZVAL_NULL(&vnull);
    params = safe_emalloc(sizeof(zval), 2, 0);
    ZVAL_COPY_VALUE(&params[0], &knull);
    ZVAL_COPY_VALUE(&params[1], &vnull);

    ZVAL_STRINGL(&method, "__construct", strlen("__construct"));
    call_user_function(NULL, &_itemval, &method, &retval, 2, params TSRMLS_CC);
    call_user_function(NULL, &itemval, &method, &retval, 2, params TSRMLS_CC);


    if (Z_TYPE(itemval) == IS_OBJECT) {
        zend_update_property(buffer_pool_ce, self, ZEND_STRL("head"), &itemval);
        zend_update_property(buffer_pool_ce, self, ZEND_STRL("tail"), &itemval);
    }

    zval *head = zend_read_property(buffer_pool_ce, self, ZEND_STRL("head"), slient, &rv);
    zval *tail = zend_read_property(buffer_pool_ce, self, ZEND_STRL("tail"), slient, &rv);
    if (Z_TYPE_P(head) == IS_OBJECT &&
        Z_TYPE_P(tail) == IS_OBJECT
    ) {
        zend_update_property(buffer_pool_ce, head, ZEND_STRL("next"), &_itemval);
        zend_update_property(buffer_pool_ce, tail, ZEND_STRL("prev"), &_itemval);
        if (Z_TYPE(_itemval) != IS_OBJECT) {
            php_printf("%s", "throw exception");
            RETURN_FALSE;
        }
    }


    RETURN_LONG(1);
}



PHP_METHOD(buffer_pool, clear)
{
    zval rv = {{0}};
    zend_bool slient;
    zval hashmap, *head, *tail;

    zval vnull;
    ZVAL_NULL(&vnull);
    array_init(&hashmap);

    zend_update_property(buffer_pool_ce, getThis(), ZEND_STRL("hashmap"), &hashmap);
    zend_update_property(buffer_pool_ce, getThis(), ZEND_STRL("head"), &vnull);
    zend_update_property(buffer_pool_ce, getThis(), ZEND_STRL("tail"), &vnull);

    RETURN_LONG(1);
}



PHP_METHOD(buffer_pool, get)
{
    zend_string *key;
    zval *hashmap, *node;
    zval method, retval, rv;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "S", &key) != SUCCESS) {
        return;
    }

    hashmap = zend_read_property(buffer_pool_ce, getThis(), ZEND_STRL("hashmap"), 1, &rv);
    if (!zend_hash_exists(Z_ARRVAL_P(hashmap), key)) {
        RETURN_FALSE;
    }

    node = zend_hash_find(Z_ARRVAL_P(hashmap), key);
    if (Z_TYPE_P(node) != IS_OBJECT) {
        RETURN_FALSE;
    }

    int optionCount = zend_hash_num_elements(Z_ARRVAL_P(hashmap));
    if (optionCount > 0) {
        php_printf("%s", "attach-detach-1");
    }

    ZVAL_STRINGL(&method, "getEntity", strlen("getEntity"));
    call_user_function(NULL, node, &method, &retval, 0, NULL TSRMLS_CC);

    RETURN_STRING(Z_STRVAL(retval));
}



PHP_METHOD(buffer_pool, set)
{
    int expired;
    zend_string *key, *value;
    zval *capacity, *self, *node, *hashmap, *params;
    zval method, retval, itemval;
    zval rv = {{0}};

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "SS|l", &key, &value, &expired) != SUCCESS) {
        return;
    }

    self = getThis();
    capacity = zend_read_property(buffer_pool_ce, self, ZEND_STRL("capacity"), 1, &rv);
    if (0 >= capacity) {
        php_printf("%s", "hi");
        RETURN_FALSE;
    }

    hashmap = zend_read_property(buffer_pool_ce, self, ZEND_STRL("hashmap"), 1, &rv);
    if (zend_hash_exists(Z_ARRVAL_P(hashmap), key)) {
        node = zend_hash_find(Z_ARRVAL_P(hashmap), key);
        if (Z_TYPE_P(node) == IS_OBJECT) {
            zval simple;

            ZVAL_STRINGL(&method, "setEntity", strlen("setEntity"));
            ZVAL_STRINGL(&simple, ZSTR_VAL(value), ZSTR_LEN(value));
            call_user_function(NULL, node, &method, &retval, 1, &simple TSRMLS_CC);

            php_printf("%s", "attach-detach-2");
            RETURN_TRUE;
        }
    }

    zval kval, vval, ttl;
    object_init_ex(&itemval, buffer_item_ce);
    if (Z_TYPE(itemval) == IS_OBJECT) {
        ZVAL_LONG(&ttl, expired);
        ZVAL_STRINGL(&kval, ZSTR_VAL(key), ZSTR_LEN(key));
        ZVAL_STRINGL(&vval, ZSTR_VAL(value), ZSTR_LEN(value));
        params = safe_emalloc(sizeof(zval), 3, 0);
        ZVAL_COPY_VALUE(&params[0], &kval);
        ZVAL_COPY_VALUE(&params[1], &vval);
        ZVAL_COPY_VALUE(&params[2], &ttl);

        ZVAL_STRINGL(&method, "__construct", strlen("__construct"));
        call_user_function(NULL, &itemval, &method, &retval, 3, params TSRMLS_CC);
    }

    add_assoc_zval_ex(hashmap, ZSTR_VAL(key), ZSTR_LEN(key), &itemval);
    zend_update_property(buffer_pool_ce, self, ZEND_STRL("hashmap"), hashmap);

    RETURN_STRING("attach-release-3");
}



PHP_METHOD(buffer_pool, release)
{
    int optionCount;
    zval *capacity, *hashmap, *self, *tail;
    zval method, retval, retval2, rv;


    self = getThis();
    capacity = zend_read_property(buffer_pool_ce, self, ZEND_STRL("capacity"), 1, &rv);
    hashmap = zend_read_property(buffer_pool_ce, self, ZEND_STRL("hashmap"), 1, &rv);

    optionCount = zend_hash_num_elements(Z_ARRVAL_P(hashmap));
    if (Z_LVAL_P(capacity) < optionCount) {
        tail = zend_read_property(buffer_pool_ce, self, ZEND_STRL("tail"), 1, &rv);
        ZVAL_STRINGL(&method, "getPrev", strlen("getPrev"));
        call_user_function(NULL, tail, &method, &retval, 0, NULL TSRMLS_CC);
    }

    if (Z_TYPE(retval) == IS_OBJECT) {
        php_printf("%s", "detach-4");
        ZVAL_STRINGL(&method, "getKey", strlen("getKey"));
        call_user_function(NULL, &retval, &method, &retval2, 0, NULL TSRMLS_CC);
        if (Z_STRVAL(retval2)) {
            zend_hash_str_del(Z_ARRVAL_P(hashmap), Z_STRVAL(retval2), Z_STRLEN(retval2));
        }
    }

    RETURN_LONG(1);
}


PHP_METHOD(buffer_pool, attach)
{
}


PHP_METHOD(buffer_pool, detach)
{
}


PHP_METHOD(buffer_pool, checkout)
{

}


PHP_METHOD(buffer_pool, delete)
{
    zend_string *key;
    zval *hashmap, *node;
    zval method, retval, rv;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "S", &key) != SUCCESS) {
        return;
    }

    hashmap = zend_read_property(buffer_pool_ce, getThis(), ZEND_STRL("hashmap"), 1, &rv);
    if (!zend_hash_exists(Z_ARRVAL_P(hashmap), key)) {
        RETURN_FALSE;
    }

    node = zend_hash_find(Z_ARRVAL_P(hashmap), key);
    if (Z_TYPE_P(node) != IS_OBJECT) {
        RETURN_FALSE;
    }

    RETURN_BOOL(zend_hash_del(Z_ARRVAL_P(hashmap), key));
}


PHP_METHOD(buffer_pool, size)
{
    int optionCount;
    zval *hashmap, rv;

    hashmap = zend_read_property(buffer_pool_ce, getThis(), ZEND_STRL("hashmap"), 1, &rv);
    optionCount = zend_hash_num_elements(Z_ARRVAL_P(hashmap));

    RETURN_LONG(optionCount);
}


PHP_METHOD(buffer_pool, proto)
{
    zval *hashmap, rv;
    hashmap = zend_read_property(buffer_pool_ce, getThis(), ZEND_STRL("hashmap"), 1, &rv);

    RETURN_ZVAL(hashmap, 1, 0);
}


PHP_METHOD(buffer_pool, __destruct)
{
}


const zend_function_entry pool_methods[] = {
    PHP_ME(buffer_pool, __construct, NULL, ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
    PHP_ME(buffer_pool, clear, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(buffer_pool, get, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(buffer_pool, set, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(buffer_pool, release, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(buffer_pool, attach, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(buffer_pool, detach, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(buffer_pool, checkout, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(buffer_pool, delete, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(buffer_pool, size, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(buffer_pool, proto, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(buffer_pool, __destruct, NULL, ZEND_ACC_PUBLIC)
	{NULL, NULL, NULL}	/* Must be the last line in buffer_functions[] */
};


BUFFER_MINIT_FUNCTION(pool)
{
    zend_class_entry ce;

    INIT_CLASS_ENTRY(ce, "Pool", pool_methods);
    buffer_pool_ce = zend_register_internal_class(&ce TSRMLS_CC);

    zend_declare_property_null(buffer_pool_ce, ZEND_STRL("hashmap"), ZEND_ACC_PUBLIC TSRMLS_CC);
    zend_declare_property_null(buffer_pool_ce, ZEND_STRL("head"), ZEND_ACC_PUBLIC TSRMLS_CC);
    zend_declare_property_null(buffer_pool_ce, ZEND_STRL("tail"), ZEND_ACC_PUBLIC TSRMLS_CC);
    zend_declare_property_null(buffer_pool_ce, ZEND_STRL("capacity"), ZEND_ACC_PUBLIC TSRMLS_CC);

    BUFFER_STARTUP(item);

	return SUCCESS;
}

