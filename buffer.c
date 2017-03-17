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
#include "ext/standard/info.h"
#include "php_buffer.h"
#include "zend.h"
#include "Zend/zend_API.h"
#include "zend_portability.h"
#include "Zend/zend_operators.h"
#include "ext/standard/php_var.h"
#include "ext/standard/php_string.h"
#include "ext/standard/php_array.h"


static int le_buffer;
zend_class_entry *buffer_ce;


static char *microtime()
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

    zend_update_property_str(buffer_ce, self, ZEND_STRL("key"), key);
    zend_update_property(buffer_ce, self, ZEND_STRL("createTime"), &str);
    zend_update_property_long(buffer_ce, self, ZEND_STRL("expireTime"), expires);
    zend_update_property_str(buffer_ce, self, ZEND_STRL("entity"), value);
    zend_update_property_long(buffer_ce, self, ZEND_STRL("hitCount"), 1);

    return (int) SUCCESS;
}


PHP_METHOD(bufItem, __construct)
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


PHP_METHOD(bufItem, isExpired)
{
    zval str;
    ZVAL_STRINGL(&str, microtime(), strlen(microtime()));
    convert_to_long(&str);

    zend_bool slient;
    zval *expireTime,
         *createTime,
         rv;

    expireTime = zend_read_property(buffer_ce, getThis(), ZEND_STRL("expireTime"), slient, &rv);
    createTime = zend_read_property(buffer_ce, getThis(), ZEND_STRL("createTime"), slient, &rv);
    int valid = (Z_LVAL_P(expireTime) != -1 && (Z_LVAL(str) - Z_LVAL_P(createTime)) > Z_LVAL_P(expireTime));
    if (valid) {
        RETVAL_LONG(1);
        return;
    }

    RETVAL_LONG(0);
}


PHP_METHOD(bufItem, setPrev)
{
    zval *value;

    zval *self = getThis();
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &value) == FAILURE) {
        return;
    } else {
        zend_update_property(buffer_ce, self, ZEND_STRL("prev"), value);
    }

    RETVAL_BOOL(1);
}


PHP_METHOD(bufItem, getPrev)
{
    zend_bool slient;
    zval *value;
    zval rv;

    zval *self = getThis();
    value = zend_read_property(buffer_ce, self, ZEND_STRL("prev"), slient, &rv);

    RETVAL_STRING(Z_STRVAL_P(value));
}


PHP_METHOD(bufItem, setNext)
{
    zval *value;

    zval *self = getThis();
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &value) == FAILURE) {
        return;
    } else {
        zend_update_property(buffer_ce, self, ZEND_STRL("next"), value);
    }

    RETVAL_BOOL(1);
}


PHP_METHOD(bufItem, getNext)
{
    zend_bool slient;
    zval *value;
    zval rv;

    zval *self = getThis();
    value = zend_read_property(buffer_ce, self, ZEND_STRL("next"), slient, &rv);

    RETVAL_STRING(Z_STRVAL_P(value));
}


PHP_METHOD(bufItem, setEntity)
{
    zend_bool slient;
    zval *entity, *hotCount, *value;
    zval rv;

    zval *self = getThis();
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &value) == FAILURE) {
        return;
    } else {
        zend_update_property(buffer_ce, self, ZEND_STRL("entity"), value);
    }

    hotCount = zend_read_property(buffer_ce, self, ZEND_STRL("hitCount"), slient, &rv);
    if (Z_LVAL_P(hotCount) > 0)
    {
        long newval;
        newval = Z_LVAL_P(hotCount) + 1;
        ZVAL_LONG(hotCount, newval)
        zend_update_property(buffer_ce, self, ZEND_STRL("hitCount"), hotCount);
    }

    RETVAL_BOOL(1);
}


PHP_METHOD(bufItem, getEntity)
{
    zend_bool slient;
    zval *entity, *hotCount;
    zval rv;

    zval *self = getThis();
    entity = zend_read_property(buffer_ce, self, ZEND_STRL("entity"), slient, &rv);
    hotCount = zend_read_property(buffer_ce, self, ZEND_STRL("hitCount"), slient, &rv);
    if (Z_LVAL_P(hotCount) > 0)
    {
        long newval;
        newval = Z_LVAL_P(hotCount) + 1;
        ZVAL_LONG(hotCount, newval);
        zend_update_property(buffer_ce, self, ZEND_STRL("hitCount"), hotCount);
    }

    RETVAL_STRING(Z_STRVAL_P(entity));
}


PHP_METHOD(bufItem, getKey)
{
    zend_bool slient;
    zval *value;
    zval rv;

    zval *self = getThis();
    value = zend_read_property(buffer_ce, self, ZEND_STRL("key"), slient, &rv);

    RETVAL_STRING(Z_STRVAL_P(value));
}


PHP_METHOD(bufItem, getHitCount)
{
    zend_bool slient;
    zval *value;
    zval rv;

    zval *self = getThis();
    value = zend_read_property(buffer_ce, self, ZEND_STRL("hitCount"), slient, &rv);

    RETVAL_LONG(Z_LVAL_P(value));
}


PHP_METHOD(bufItem, getTime)
{
    zend_bool slient;
    zval *value;
    zval rv;

    zval *self = getThis();
    value = zend_read_property(buffer_ce, self, ZEND_STRL("createTime"), slient, &rv);

    RETVAL_LONG(Z_LVAL_P(value));
}


PHP_METHOD(bufItem, __invoke)
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

    RETVAL_NULL();
}



const zend_function_entry demo_methods[] = {
    PHP_ME(bufItem, __construct, NULL, ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
    PHP_ME(bufItem, isExpired, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(bufItem, setPrev, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(bufItem, getPrev, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(bufItem, setNext, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(bufItem, getNext, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(bufItem, setEntity, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(bufItem, getEntity, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(bufItem, getKey, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(bufItem, getHitCount, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(bufItem, getTime, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(bufItem, __invoke, NULL, ZEND_ACC_PUBLIC)
	{NULL, NULL, NULL}	/* Must be the last line in buffer_functions[] */
};


PHP_MINIT_FUNCTION(buffer)
{
    zend_class_entry ce;

    INIT_CLASS_ENTRY(ce, "bufItem", demo_methods);
    buffer_ce = zend_register_internal_class(&ce TSRMLS_CC);

    zend_declare_property_null(buffer_ce, ZEND_STRL("key"), ZEND_ACC_PUBLIC TSRMLS_CC);
    zend_declare_property_null(buffer_ce, ZEND_STRL("prev"), ZEND_ACC_PUBLIC TSRMLS_CC);
    zend_declare_property_null(buffer_ce, ZEND_STRL("next"), ZEND_ACC_PUBLIC TSRMLS_CC);
    zend_declare_property_null(buffer_ce, ZEND_STRL("createTime"), ZEND_ACC_PUBLIC TSRMLS_CC);
    zend_declare_property_null(buffer_ce, ZEND_STRL("expireTime"), ZEND_ACC_PUBLIC TSRMLS_CC);
    zend_declare_property_null(buffer_ce, ZEND_STRL("entity"), ZEND_ACC_PUBLIC TSRMLS_CC);
    zend_declare_property_null(buffer_ce, ZEND_STRL("hitCount"), ZEND_ACC_PUBLIC TSRMLS_CC);

	return SUCCESS;
}


PHP_MSHUTDOWN_FUNCTION(buffer)
{
	return SUCCESS;
}


zend_module_entry buffer_module_entry = {
	STANDARD_MODULE_HEADER,
	"buffer",
	NULL,
	PHP_MINIT(buffer),
	PHP_MSHUTDOWN(buffer),
	NULL,    /* Replace with NULL if there's nothing to do at request start */
	NULL,	/* Replace with NULL if there's nothing to do at request end */
	NULL,
	PHP_BUFFER_VERSION,
	STANDARD_MODULE_PROPERTIES
};


#ifdef COMPILE_DL_BUFFER
#ifdef ZTS
ZEND_TSRMLS_CACHE_DEFINE()
#endif
ZEND_GET_MODULE(buffer)
#endif

