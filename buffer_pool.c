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
#include "ext/standard/php_array.h"


zend_class_entry *buffer_pool_ce;


/** {{{ ARG_INFO
 *  */
ZEND_BEGIN_ARG_INFO_EX(buffer_pool_void_arginfo, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(buffer_pool_construct_arginfo, 0, 0, 1)
	ZEND_ARG_INFO(0, capacity)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(buffer_pool_get_arginfo, 0, 0, 1)
	ZEND_ARG_INFO(0, key)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(buffer_pool_set_arginfo, 0, 0, 3)
	ZEND_ARG_INFO(0, key)
	ZEND_ARG_INFO(0, value)
	ZEND_ARG_INFO(0, expired)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(buffer_pool_attach_arginfo, 0, 0, 2)
	ZEND_ARG_INFO(0, head)
	ZEND_ARG_INFO(0, node)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(buffer_pool_detach_arginfo, 0, 0, 1)
	ZEND_ARG_INFO(0, node)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(buffer_pool_delete_arginfo, 0, 0, 1)
	ZEND_ARG_INFO(0, key)
ZEND_END_ARG_INFO()
/* }}} */


static zval call_user_func_array(zval *object, char *method_name, int paramlen, zval *paramval[3])
{
    zval retval, method, *params;
    if (Z_TYPE_P(object) == IS_STRING || Z_TYPE_P(object) == IS_ARRAY) {
        ZVAL_NULL(&retval);
        return (zval) retval;
    } else if (Z_TYPE_P(object) == IS_NULL) {
        object = NULL;
    }

    if (1 == paramlen) {
        ZVAL_STRINGL(&method, method_name, strlen(method_name));
        call_user_function(EG(function_table), object, &method, &retval, 1, *paramval TSRMLS_CC);
    } else if (2 == paramlen) {
        params = safe_emalloc(sizeof(zval), 2, 0);
        ZVAL_COPY_VALUE(&params[0], paramval[0]);
        ZVAL_COPY_VALUE(&params[1], paramval[1]);
        ZVAL_STRINGL(&method, method_name, strlen(method_name));
        call_user_function(EG(function_table), object, &method, &retval, 2, params TSRMLS_CC);
    } else if (3 == paramlen) {
        params = safe_emalloc(sizeof(zval), 3, 0);
        ZVAL_COPY_VALUE(&params[0], paramval[0]);
        ZVAL_COPY_VALUE(&params[1], paramval[1]);
        ZVAL_COPY_VALUE(&params[2], paramval[2]);
        ZVAL_STRINGL(&method, method_name, strlen(method_name));
        call_user_function(EG(function_table), object, &method, &retval, 3, params TSRMLS_CC);
    } else {
        ZVAL_STRINGL(&method, method_name, strlen(method_name));
        call_user_function(EG(function_table), object, &method, &retval, 0, NULL TSRMLS_CC);
    }

    return (zval) retval;
}


static zval __construct(int paramlen, zend_string *key, zend_string *value, int expired)
{
    zval *params[paramlen];
    zval itemval;
    zval kval, vval, ttl;

    object_init_ex(&itemval, buffer_item_ce);
    if (paramlen > 2) {
        ZVAL_LONG(&ttl, expired);
        ZVAL_STRINGL(&kval, ZSTR_VAL(key), ZSTR_LEN(key));
        ZVAL_STRINGL(&vval, ZSTR_VAL(value), ZSTR_LEN(value));
        params[0] = (zval *)&kval;
        params[1] = (zval *)&vval;
        params[2] = (zval *)&ttl;
        call_user_func_array(&itemval, "__construct", 3, params);
    } else {
        ZVAL_NULL(&kval);
        ZVAL_NULL(&vval);
        params[0] = (zval *)&kval;
        params[1] = (zval *)&vval;
        call_user_func_array(&itemval, "__construct", 2, params);
    }

    return (zval) itemval;
}


PHP_METHOD(buffer_pool, __construct)
{
    int capacity;
    zval *self, *params[2];
    zval retval, map, itemval;
    zend_bool slient;
    zval rv = {{0}};


    self = getThis();
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &capacity) == FAILURE) {
        return;
    } else {
        array_init(&map);
        zend_update_property(buffer_pool_ce, self, ZEND_STRL("hashmap"), &map);
        zend_update_property_long(buffer_pool_ce, self, ZEND_STRL("capacity"), capacity);
    }

    // init __construct function
    itemval = __construct(2, NULL, NULL, 0);
    if (Z_TYPE(itemval) == IS_OBJECT) {
        zend_update_property(buffer_pool_ce, self, ZEND_STRL("head"), &itemval);
        zend_update_property(buffer_pool_ce, self, ZEND_STRL("tail"), &itemval);
    }

    zval *head = zend_read_property(buffer_pool_ce, self, ZEND_STRL("head"), slient, &rv);
    zval *tail = zend_read_property(buffer_pool_ce, self, ZEND_STRL("tail"), slient, &rv);
    if (Z_TYPE_P(head) == IS_OBJECT) {
        zend_update_property(buffer_pool_ce, head, ZEND_STRL("next"), &itemval);
    }

    if (Z_TYPE_P(tail) == IS_OBJECT) {
        zend_update_property(buffer_pool_ce, tail, ZEND_STRL("prev"), &itemval);
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

    if (Z_TYPE(hashmap) == IS_ARRAY) {
        zend_update_property(buffer_pool_ce, getThis(), ZEND_STRL("hashmap"), &hashmap);
        zend_update_property(buffer_pool_ce, getThis(), ZEND_STRL("head"), &vnull);
        zend_update_property(buffer_pool_ce, getThis(), ZEND_STRL("tail"), &vnull);
    }

    RETURN_LONG(1);
}



PHP_METHOD(buffer_pool, get)
{
    zend_string *key;
    zval *hashmap, *node, *head;
    zval *params[2], *self;
    zval method, retval, rv;

    self = getThis();
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "S", &key) != SUCCESS) {
        return;
    }

    hashmap = zend_read_property(buffer_pool_ce, self, ZEND_STRL("hashmap"), 1, &rv);
    if (!zend_hash_exists(Z_ARRVAL_P(hashmap), key)) {
        RETURN_FALSE;
    }

    node = zend_hash_find(Z_ARRVAL_P(hashmap), key);
    if (Z_TYPE_P(node) != IS_OBJECT) {
        RETURN_FALSE;
    }

    int optionCount = zend_hash_num_elements(Z_ARRVAL_P(hashmap));
    if (optionCount > 0) {
        retval = call_user_func_array(self, "detach", 1, &node);
        if (Z_TYPE(retval) == IS_OBJECT) {
            head = zend_read_property(buffer_pool_ce, self, ZEND_STRL("head"), 1, &rv);
            params[0] = head;
            params[1] = node;
            call_user_func_array(self, "attach", 2, params);
        }
    }

    retval = call_user_func_array(node, "getEntity", 0, NULL);
    RETURN_STRING(Z_STRVAL(retval));
}



PHP_METHOD(buffer_pool, set)
{
    int expired;
    zend_string *key, *value;
    zval *capacity, *self, *node, *hashmap, *head, *params[3];
    zval retval, itemval, simple;
    zval rv = {{0}};

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "SS|l", &key, &value, &expired) != SUCCESS) {
        return;
    }

    self = getThis();
    capacity = zend_read_property(buffer_pool_ce, self, ZEND_STRL("capacity"), 1, &rv);
    if (0 >= capacity) {
        php_printf("%s", "The capacity cannot for null");
        RETURN_FALSE;
    }

    hashmap = zend_read_property(buffer_pool_ce, self, ZEND_STRL("hashmap"), 1, &rv);
    if (!zend_hash_exists(Z_ARRVAL_P(hashmap), key)) {
        // init __consturct function
        itemval = __construct(3, key, value, expired);

        add_assoc_zval_ex(hashmap, ZSTR_VAL(key), ZSTR_LEN(key), &itemval);
        zend_update_property(buffer_pool_ce, self, ZEND_STRL("hashmap"), hashmap);

        head = zend_read_property(buffer_pool_ce, self, ZEND_STRL("head"), 1, &rv);
        params[0] = (zval *)head;
        params[1] = (zval *)&itemval;
        call_user_func_array(self, "attach", 2, params);

        // return release
        retval = call_user_func_array(self, "release", 0, NULL);
    } else {
        node = zend_hash_find(Z_ARRVAL_P(hashmap), key);
        if (Z_TYPE_P(node) == IS_OBJECT) {
            ZVAL_STRINGL(&simple, ZSTR_VAL(value), ZSTR_LEN(value));
            call_user_func_array(node, "setEntity", 1, (zval **)&simple);

            call_user_func_array(self, "detach", 1, &node);
            head = zend_read_property(buffer_pool_ce, self, ZEND_STRL("head"), 1, &rv);
            params[0] = head;
            params[1] = node;
            call_user_func_array(self, "attach", 2, params);
            RETURN_BOOL(1);
        }
    }

    RETURN_BOOL(Z_TYPE(retval));
}



PHP_METHOD(buffer_pool, release)
{
    int optionCount;
    zval *capacity, *hashmap, method, *self, *tail;
    zval *prev, *key;
    zval rv = {{0}};

    self = getThis();
    capacity = zend_read_property(buffer_pool_ce, self, ZEND_STRL("capacity"), 1, &rv);
    hashmap = zend_read_property(buffer_pool_ce, self, ZEND_STRL("hashmap"), 1, &rv);

    optionCount = zend_hash_num_elements(Z_ARRVAL_P(hashmap));
    if (Z_LVAL_P(capacity) < optionCount) {
        tail = zend_read_property(buffer_pool_ce, self, ZEND_STRL("tail"), 1, &rv);
        prev = zend_read_property(buffer_pool_ce, tail, ZEND_STRL("prev"), 1, &rv);
        if (Z_TYPE_P(prev) == IS_OBJECT) {
            call_user_func_array(self, "detach", 1, &prev);
            key = zend_read_property(buffer_pool_ce, prev, ZEND_STRL("key"), 1, &rv);
            if (Z_STRVAL_P(key)) {
                zend_hash_str_del(Z_ARRVAL_P(hashmap), Z_STRVAL_P(key), Z_STRLEN_P(key));
                RETURN_BOOL(1);
            }
        }
    }

    RETURN_BOOL(0);
}


PHP_METHOD(buffer_pool, attach)
{
    zval *self, *head, *node, *next, *prev;
    zval method, rv;
    zval retval;

    self = getThis();
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "oo", &head, &node) != SUCCESS) {
        return;
    }

    if (Z_TYPE_P(node) != IS_OBJECT) {
        RETURN_FALSE;
    }

    // node setPrev head
    call_user_func_array(node, "setPrev", 1, &head);
    next = zend_read_property(buffer_pool_ce, head, ZEND_STRL("next"), 1, &rv);
    if (Z_TYPE_P(next) != IS_NULL) {
        call_user_func_array(node, "setNext", 1, &next);
    }

    next = zend_read_property(buffer_pool_ce, node, ZEND_STRL("next"), 1, &rv);
    prev = zend_read_property(buffer_pool_ce, node, ZEND_STRL("prev"), 1, &rv);
    if (Z_TYPE_P(next) != IS_NULL) {
        call_user_func_array(next, "setPrev", 1, &node);
    }

    if (Z_TYPE_P(prev) != IS_NULL) {
        retval = call_user_func_array(prev, "setNext", 1, &node);
    }

    RETURN_ZVAL(getThis(), 1, 0);
}


PHP_METHOD(buffer_pool, detach)
{
    zval *self, *node, *prev, *next;
    zval method, rv;

    self = getThis();
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "o", &node) != SUCCESS) {
        return;
    }

    if (Z_TYPE_P(node) != IS_OBJECT) {
        RETURN_FALSE;
    }

    prev = zend_read_property(buffer_pool_ce, node, ZEND_STRL("prev"), 1, &rv);
    next = zend_read_property(buffer_pool_ce, node, ZEND_STRL("next"), 1, &rv);
    if (Z_TYPE_P(prev) != IS_NULL) {
        call_user_func_array(prev, "setNext", 1, &next);
    }

    if (Z_TYPE_P(next) != IS_NULL) {
        call_user_func_array(next, "setPrev", 1, &prev);
    }

    RETURN_ZVAL(getThis(), 1, 0);
}


PHP_METHOD(buffer_pool, checkout)
{
    zval key, method;
    zval *hashmap;
    zval rv = {{0}};

    hashmap = zend_read_property(buffer_pool_ce, getThis(), ZEND_STRL("hashmap"), 1, &rv);
    ZVAL_NULL(&method);
    key = call_user_func_array(&method, "array_rand", 1, &hashmap);
    if (Z_STRLEN(key) == 0) {
        RETURN_FALSE;
    }

    RETURN_STRING(Z_STRVAL(key));
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
    PHP_ME(buffer_pool, __construct, buffer_pool_construct_arginfo, ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
    PHP_ME(buffer_pool, clear, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(buffer_pool, get, buffer_pool_get_arginfo, ZEND_ACC_PUBLIC)
    PHP_ME(buffer_pool, set, buffer_pool_set_arginfo, ZEND_ACC_PUBLIC)
    PHP_ME(buffer_pool, release, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(buffer_pool, attach, buffer_pool_attach_arginfo, ZEND_ACC_PUBLIC)
    PHP_ME(buffer_pool, detach, buffer_pool_detach_arginfo, ZEND_ACC_PUBLIC)
    PHP_ME(buffer_pool, checkout, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(buffer_pool, delete, buffer_pool_delete_arginfo, ZEND_ACC_PUBLIC)
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

    zend_declare_property_null(buffer_pool_ce, ZEND_STRL("hashmap"), ZEND_ACC_PROTECTED TSRMLS_CC);
    zend_declare_property_null(buffer_pool_ce, ZEND_STRL("head"), ZEND_ACC_PROTECTED TSRMLS_CC);
    zend_declare_property_null(buffer_pool_ce, ZEND_STRL("tail"), ZEND_ACC_PROTECTED TSRMLS_CC);
    zend_declare_property_null(buffer_pool_ce, ZEND_STRL("capacity"), ZEND_ACC_PROTECTED TSRMLS_CC);

    BUFFER_STARTUP(item);

	return SUCCESS;
}

