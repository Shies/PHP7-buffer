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
#include "Zend/zend_alloc.h"
#include "ext/standard/info.h"
#include "ext/standard/php_string.h"

#include "php_buffer.h"
#include "buffer_item.h"
#include "buffer_pool.h"
#include "buffer_org.h"


static int le_buffer;
zend_class_entry *buffer_ce;


PHP_INI_BEGIN()
	PHP_INI_ENTRY("buffer.enable", "1",  PHP_INI_ALL, NULL)
PHP_INI_END();


const zend_function_entry buffer_methods[] = {
	{NULL, NULL, NULL}	/* Must be the last line in buffer_functions[] */
};


PHP_MINIT_FUNCTION(buffer)
{
    REGISTER_INI_ENTRIES();

    BUFFER_STARTUP(item);
    BUFFER_STARTUP(pool);
    BUFFER_STARTUP(org);

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

