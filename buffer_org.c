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
#include "buffer_org.h"
#include "ext/standard/info.h"
#include "ext/standard/php_var.h"
#include "ext/standard/php_string.h"


zend_class_entry *buffer_org_ce;


PHP_METHOD(buffer_org, __construct)
{
    zval map = {{0}};
    array_init(&map);
    zend_update_static_property(buffer_org_ce, ZEND_STRL("pool"), &map);
}


PHP_METHOD(buffer_org, getInstance)
{
    zval org = {{0}};
    zval method, retval;
    zval *instance;
    zend_bool slient;

    instance = zend_read_static_property(buffer_org_ce, ZEND_STRL("instance"), slient);
    if (IS_OBJECT == Z_TYPE_P(instance)) {
        RETURN_ZVAL(instance, 1, 0);
    }

    if (Z_ISUNDEF(org)) {
        object_init_ex(&org, buffer_org_ce);
        ZVAL_STRINGL(&method, "__construct", strlen("__construct"));
        call_user_function(NULL, &org, &method, &retval, 0, NULL TSRMLS_CC);
    }
    zend_update_static_property(buffer_org_ce, ZEND_STRL("instance"), &org);


    RETURN_ZVAL(&org, 1, 0);
}


PHP_METHOD(buffer_org, load)
{
    zend_string *name;
    zval *poolmap, *params;
    zval pool, vnull, method, retval;
    zend_bool slient;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "S", &name) == FAILURE) {
        return;
    }

    if (!ZSTR_VAL(name)) {
        RETURN_FALSE;
    }

    poolmap = zend_read_static_property(buffer_org_ce, ZEND_STRL("pool"), slient);
    if (zend_hash_exists(Z_ARRVAL_P(poolmap), name)) {
        RETURN_ZVAL(poolmap, 1, 0);
    }

    object_init_ex(&pool, buffer_pool_ce);

    ZVAL_LONG(&vnull, 10);
    ZVAL_STRINGL(&method, "__construct", strlen("__construct"));
    call_user_function(NULL, &pool, &method, &retval, 1, &vnull TSRMLS_CC);
    if (Z_TYPE(pool) == IS_OBJECT) {
        add_assoc_zval_ex(poolmap, ZSTR_VAL(name), ZSTR_LEN(name), &pool);
        zend_update_static_property(buffer_org_ce, ZEND_STRL("pool"), poolmap);
    }


    RETURN_ZVAL(poolmap, 1, 0);
}


PHP_METHOD(buffer_org, getAdapter)
{
    zval rv;
    zval *adapter;
    zend_bool slient;

    adapter = zend_read_property(buffer_org_ce, getThis(), ZEND_STRL("adapter"), slient, &rv);
    RETURN_ZVAL(adapter, 1, 0);
}


PHP_METHOD(buffer_org, getPool)
{
    zval *poolmap;
    zend_bool slient;

    poolmap = zend_read_static_property(buffer_org_ce, ZEND_STRL("pool"), slient);
    RETURN_ZVAL(poolmap, 1, 0);
}


const zend_function_entry org_methods[] = {
    PHP_ME(buffer_org, __construct, NULL, ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
    PHP_ME(buffer_org, getInstance, NULL, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
    PHP_ME(buffer_org, load, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(buffer_org, getAdapter, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(buffer_org, getPool, NULL, ZEND_ACC_PUBLIC)
	{NULL, NULL, NULL}	/* Must be the last line in buffer_functions[] */
};


BUFFER_MINIT_FUNCTION(org)
{
    zend_class_entry ce;

    INIT_CLASS_ENTRY(ce, "OrgManager", org_methods);
    buffer_org_ce = zend_register_internal_class(&ce TSRMLS_CC);

    zend_declare_property_null(buffer_org_ce, ZEND_STRL("instance"), ZEND_ACC_PUBLIC|ZEND_ACC_STATIC TSRMLS_CC);
    zend_declare_property_null(buffer_org_ce, ZEND_STRL("pool"), ZEND_ACC_PUBLIC|ZEND_ACC_STATIC TSRMLS_CC);
    zend_declare_property_null(buffer_org_ce, ZEND_STRL("adapter"), ZEND_ACC_PUBLIC TSRMLS_CC);

    BUFFER_STARTUP(item);
    BUFFER_STARTUP(pool);

	return SUCCESS;
}

