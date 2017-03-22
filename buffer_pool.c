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
#include "buffer_pool.h"
#include "ext/standard/info.h"
#include "ext/standard/php_string.h"

zend_class_entry *buffer_pool_ce;


PHP_METHOD(buffer_pool, __construct)
{
    int capacity;
    zval *self,
         *itemval,
         *construct;

    self = getThis();
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &capacity) == FAILURE) {
        return;
    } else {
        zend_update_property_long(buffer_pool_ce, self, ZEND_STRL("capacity"), capacity);
    }


    php_printf("%s", "hello");
    return;

    /*
    params = safe_emalloc(sizeof(zval), 2, 0);
    ZVAL_COPY_VALUE(&params[0], (zval *)NULL);
    ZVAL_COPY_VALUE(&params[1], (zval *)NULL);
    call_user_function(NULL, itemval, method_construct, &retval, 2, params TSRMLS_CC);


    if (Z_TYPE_P(itemval) == IS_OBJECT) {
        php_printf("%s", "b1");
        return;
        zend_update_property(buffer_pool_ce, self, ZEND_STRL("head"), itemval);
        zend_update_property(buffer_pool_ce, self, ZEND_STRL("tail"), itemval);
    }

    php_printf("%s", "b2");
    return;
    ZVAL_STRINGL(&function_name, "setNext", strlen("setNext"));
    zval *head = zend_read_property(buffer_pool_ce, self, ZEND_STRL("head"), 1, &rv);
    php_printf("%s", "b3");
    return;
    if (Z_TYPE_P(head) == IS_OBJECT) {
        ZVAL_OBJ(&args, Z_OBJ_P(head));
        call_user_function(CG(function_table), itemval, &function_name, &retval, 1, &args TSRMLS_CC);
        if (Z_TYPE(retval) != IS_OBJECT) {
            php_printf("%s", "throw exception");
            RETURN_FALSE;
        }
    }

    ZVAL_STRINGL(&function_name, "setPrev", strlen("setPrev"));
    zval *tail = zend_read_property(buffer_pool_ce, self, ZEND_STRL("tail"), 1, &rv);
    php_printf("%s", "b4");
    return;
    if (Z_TYPE_P(tail) == IS_OBJECT) {
        ZVAL_OBJ(&args, Z_OBJ_P(tail));
        call_user_function(CG(function_table), itemval, &function_name, &retval, 1, &args TSRMLS_CC);
        if (Z_TYPE(retval) != IS_OBJECT) {
            php_printf("%s", "throw exception");
            RETURN_FALSE;
        }
    }
    */

    RETURN_LONG(1);
}


const zend_function_entry pool_methods[] = {
    PHP_ME(buffer_pool, __construct, NULL, ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
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

