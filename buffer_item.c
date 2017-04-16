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
#include "Zend/zend_interfaces.h"

#include "php_buffer.h"
#include "buffer_item.h"
#include "ext/standard/info.h"
#include "ext/standard/php_string.h"


zend_class_entry *buffer_item_ce;


/** {{{ ARG_INFO
 *  */
ZEND_BEGIN_ARG_INFO_EX(buffer_item_void_arginfo, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(buffer_item_construct_arginfo, 0, 0, 3)
	ZEND_ARG_INFO(0, key)
	ZEND_ARG_INFO(0, value)
	ZEND_ARG_INFO(0, expires)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(buffer_item_setPrev_arginfo, 0, 0, 1)
	ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(buffer_item_setNext_arginfo, 0, 0, 1)
	ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(buffer_item_setEntity_arginfo, 0, 0, 1)
	ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO()
/* }}} */


static char *microtime(void)
{
    struct timeval time;
    zend_string *timestamp;
    zend_string *dot = zend_string_init(".", strlen("."), 1);

    gettimeofday(&time, NULL);
    timestamp = strpprintf(0, "%ld%s%ld", (long) time.tv_sec, ZSTR_VAL(dot), (long) time.tv_usec);

    return ZSTR_VAL(timestamp);
}


static int initVar(zval *self, zend_string *key, zend_string *value, long expires)
{
    zval str;
    ZVAL_STRINGL(&str, microtime(), strlen(microtime()));
    convert_to_long(&str);

    zend_update_property_str(buffer_item_ce, self, ZEND_STRL("key"), key);
    zend_update_property(buffer_item_ce, self, ZEND_STRL("createTime"), &str);
    zend_update_property_long(buffer_item_ce, self, ZEND_STRL("expireTime"), expires);
    zend_update_property_str(buffer_item_ce, self, ZEND_STRL("entity"), value);
    zend_update_property_long(buffer_item_ce, self, ZEND_STRL("hitCount"), 1);

    return (int) SUCCESS;
}


PHP_METHOD(buffer_item, __construct)
{
    long expires;
    zval *self;
    zend_string *key, *value;

    self = getThis();
    if (zend_parse_parameters_throw(ZEND_NUM_ARGS() TSRMLS_CC, "SS|l", &key, &value, &expires) == FAILURE) {
        return;
    }

    initVar(self, key, value, expires);
}


PHP_METHOD(buffer_item, isExpired)
{
    zval str;
    ZVAL_STRINGL(&str, microtime(), strlen(microtime()));
    convert_to_long(&str);

    zend_bool slient;
    zval *expireTime,
         *createTime,
         rv;

    expireTime = zend_read_property(buffer_item_ce, getThis(), ZEND_STRL("expireTime"), slient, &rv);
    createTime = zend_read_property(buffer_item_ce, getThis(), ZEND_STRL("createTime"), slient, &rv);
    int valid = (Z_LVAL_P(expireTime) != -1 && (Z_LVAL(str) - Z_LVAL_P(createTime)) > Z_LVAL_P(expireTime));
    if (valid) {
        RETVAL_LONG(1);
        return;
    }

    RETURN_LONG(0);
}


PHP_METHOD(buffer_item, setPrev)
{
    zval *value;

    zval *self = getThis();
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &value) == FAILURE) {
        return;
    } else {
        zend_update_property(buffer_item_ce, self, ZEND_STRL("prev"), value);
    }

    RETURN_BOOL(1);
}


PHP_METHOD(buffer_item, getPrev)
{
    zend_bool slient;
    zval *value;
    zval rv;

    zval *self = getThis();
    value = zend_read_property(buffer_item_ce, self, ZEND_STRL("prev"), slient, &rv);

    RETURN_STRING(Z_STRVAL_P(value));
}


PHP_METHOD(buffer_item, setNext)
{
    zval *value;

    zval *self = getThis();
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &value) == FAILURE) {
        return;
    } else {
        zend_update_property(buffer_item_ce, self, ZEND_STRL("next"), value);
    }

    RETURN_BOOL(1);
}


PHP_METHOD(buffer_item, getNext)
{
    zend_bool slient;
    zval *value;
    zval rv;

    zval *self = getThis();
    value = zend_read_property(buffer_item_ce, self, ZEND_STRL("next"), slient, &rv);

    RETURN_STRING(Z_STRVAL_P(value));
}


PHP_METHOD(buffer_item, setEntity)
{
    zend_bool slient;
    zval *entity, *hotCount, *value;
    zval rv;

    zval *self = getThis();
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &value) == FAILURE) {
        return;
    } else {
        zend_update_property(buffer_item_ce, self, ZEND_STRL("entity"), value);
    }

    hotCount = zend_read_property(buffer_item_ce, self, ZEND_STRL("hitCount"), slient, &rv);
    if (Z_LVAL_P(hotCount) > 0)
    {
        long newval;
        newval = Z_LVAL_P(hotCount) + 1;
        ZVAL_LONG(hotCount, newval)
        zend_update_property(buffer_item_ce, self, ZEND_STRL("hitCount"), hotCount);
    }

    RETURN_BOOL(1);
}


PHP_METHOD(buffer_item, getEntity)
{
    zend_bool slient;
    zval *entity, *hotCount;
    zval rv;

    zval *self = getThis();
    entity = zend_read_property(buffer_item_ce, self, ZEND_STRL("entity"), slient, &rv);
    hotCount = zend_read_property(buffer_item_ce, self, ZEND_STRL("hitCount"), slient, &rv);
    if (Z_LVAL_P(hotCount) > 0)
    {
        long newval;
        newval = Z_LVAL_P(hotCount) + 1;
        ZVAL_LONG(hotCount, newval);
        zend_update_property(buffer_item_ce, self, ZEND_STRL("hitCount"), hotCount);
    }

    RETURN_STRING(Z_STRVAL_P(entity));
}


PHP_METHOD(buffer_item, getKey)
{
    zend_bool slient;
    zval *value;
    zval rv;

    zval *self = getThis();
    value = zend_read_property(buffer_item_ce, self, ZEND_STRL("key"), slient, &rv);

    RETURN_STRING(Z_STRVAL_P(value));
}


PHP_METHOD(buffer_item, getHitCount)
{
    zend_bool slient;
    zval *value;
    zval rv;

    zval *self = getThis();
    value = zend_read_property(buffer_item_ce, self, ZEND_STRL("hitCount"), slient, &rv);

    RETURN_LONG(Z_LVAL_P(value));
}


PHP_METHOD(buffer_item, getTime)
{
    zend_bool slient;
    zval *value;
    zval rv;

    zval *self = getThis();
    value = zend_read_property(buffer_item_ce, self, ZEND_STRL("createTime"), slient, &rv);

    RETURN_LONG(Z_LVAL_P(value));
}


PHP_METHOD(buffer_item, __invoke)
{
    zval retval,
         function_name,
         *self = getThis();

    ZVAL_STRINGL(&function_name, "isExpired", strlen("isExpired"));
    call_user_function(NULL, self, &function_name, &retval, 0, NULL TSRMLS_CC);
    if (Z_LVAL(retval) == 1)
    {
        ZVAL_STRINGL(&function_name, "getEntity", strlen("getEntity"));
        call_user_function(NULL, self, &function_name, &retval, 0, NULL TSRMLS_CC);
        RETVAL_STRING(Z_STRVAL(retval));
        return;
    }

    RETURN_NULL();
}



const zend_function_entry item_methods[] = {
    PHP_ME(buffer_item, __construct, buffer_item_construct_arginfo, ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
    PHP_ME(buffer_item, isExpired, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(buffer_item, setPrev, buffer_item_setPrev_arginfo, ZEND_ACC_PUBLIC)
    PHP_ME(buffer_item, getPrev, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(buffer_item, setNext, buffer_item_setNext_arginfo, ZEND_ACC_PUBLIC)
    PHP_ME(buffer_item, getNext, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(buffer_item, setEntity, buffer_item_setEntity_arginfo, ZEND_ACC_PUBLIC)
    PHP_ME(buffer_item, getEntity, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(buffer_item, getKey, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(buffer_item, getHitCount, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(buffer_item, getTime, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(buffer_item, __invoke, NULL, ZEND_ACC_PUBLIC)
	{NULL, NULL, NULL}	/* Must be the last line in buffer_functions[] */
};


BUFFER_MINIT_FUNCTION(item)
{
    zend_class_entry ce;

    INIT_CLASS_ENTRY(ce, "Item", item_methods);
    buffer_item_ce = zend_register_internal_class(&ce TSRMLS_CC);

    zend_declare_property_null(buffer_item_ce, ZEND_STRL("key"), ZEND_ACC_PUBLIC TSRMLS_CC);
    zend_declare_property_null(buffer_item_ce, ZEND_STRL("prev"), ZEND_ACC_PUBLIC TSRMLS_CC);
    zend_declare_property_null(buffer_item_ce, ZEND_STRL("next"), ZEND_ACC_PUBLIC TSRMLS_CC);
    zend_declare_property_null(buffer_item_ce, ZEND_STRL("createTime"), ZEND_ACC_PROTECTED TSRMLS_CC);
    zend_declare_property_null(buffer_item_ce, ZEND_STRL("expireTime"), ZEND_ACC_PROTECTED TSRMLS_CC);
    zend_declare_property_null(buffer_item_ce, ZEND_STRL("entity"), ZEND_ACC_PROTECTED TSRMLS_CC);
    zend_declare_property_null(buffer_item_ce, ZEND_STRL("hitCount"), ZEND_ACC_PROTECTED TSRMLS_CC);

	return SUCCESS;
}

