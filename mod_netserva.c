/*
mod-netserva - Reverse Domain Path Apache2 Module

* Source: http://github.com/markc/mod-netserva
* Version: v0.0.6 20110715
* Copyright: (C) 2011 Mark Constable mc@nodality.org
* License: AGPLv3 - http://www.gnu.org/licenses/agpl-3.0.txt

Dynamically transform incoming requests for a virtual host to a
reversed domainname pathed DocumentRoot, for example;

 http://some.example.com -> WEB_PATH/com/example/some

Also apply PHP safe_mode and include_path directives specific to this
virtual host. This effectively eliminates the need to create
VirtualHostName entries for apache and along with using a wilcard DNS
alias for subdomains means that new subdomain vhosts (like www.* or
admin.* etc) can be realized by simply creating a new folder within
the vhost WEB_PATH + rdp area.

v0.0.6 20110715
Commented out the PHP safe_mode settings until the missing SUHOSIN
headers is sorted out and prepared the source for pushing into
Github.

v0.0.5 20060830
Mark Reid suggested the size of inc should be strlen(buf)+1024 to
prevent any buffer overflow attacks.

v0.0.4 20060625
Fixed a bug where (only) 16 char sites returned an incorrect path.
Providing a buffer for *inc solved this strange problem.

v0.0.3 20060620
Added new base dir for Subversion support. WPATH + rdp is the
DocumentRoot for the virtual host and SPATH + rdp is the area used
for the vhost specific subversion repository.

v0.0.2 20051218
Tidied up the code and added these comments.

v0.0.1 20050618
Initial release.

TODO:

. add some global httpd.conf directives to set WEB_PATH, SVN_PATH and
  INC_PATH dynamically.

. perhaps add a global config option to turn on/off SVN_PATH (it could
  also be used as a separate "secure" path for SSL)

*/

/* #include <stdio.h> */
#define CORE_PRIVATE

#include "httpd.h"
#include "http_config.h"
#include "http_core.h"
#include "http_log.h"
#include "http_request.h" /* for ap_hook_translate_name */ 
#include "apr_strings.h"
/*
#include "zend.h"
#include "zend_qsort.h"
#include "zend_API.h"
#include "zend_ini.h"
#include "zend_alloc.h"
#include "zend_operators.h"
#include "main/php_ini.h"
*/

#define WEB_PATH "/home/w"
/*
#define SVN_PATH "/home/s"
#define INC_PATH "/home/p:/usr/share/php:"
*/

module AP_MODULE_DECLARE_DATA netserva_module;

char *add_path_element(char *dest, char *element)
{
    *dest++ = '/';
    strcpy(dest,element);
    return dest + strlen(element);
}

static int rdp_translate(request_rec *r)
{
    const char *server_name;
    char *rdp, *p, *q, *buf, *inc;
    core_server_config * conf = (core_server_config *) ap_get_module_config(r->server->module_config, &core_module);

    /* This is basically HTTP_HOST in PHP-land */
    server_name = ap_get_server_name(r);

    /* This is the SERVER_ADMIN setting in apache-land, could be "webmaster@" */
    r->server->server_admin = apr_pstrcat(r->pool,"admin@", server_name, NULL);

    /* Copy server_name (www.domain.tld) and point buf to it */
    buf = apr_pstrdup(r->pool, server_name);

    /* Point rdp to a new buffer the same size as buf+1 */
    rdp = apr_palloc(r->pool, strlen(buf)+1);

    /* Allocate a moderately large buffer for inc to point to */
    /* This solves a corner case where 16 char server_names caused incorrect paths */
    inc = apr_palloc(r->pool, strlen(buf)+1024);

    /* Just to remove any confusion about what r->filename eventually points to */
    r->canonical_filename = "";

   /* Work out the reverse path, thanks to Mark Reid */
    *rdp = '\0';
    for (p = rdp, q = buf+strlen(buf); q >= buf; q--) {
        if (*q == '.') {
            *q = '\0';
            p = add_path_element(p, q+1);
        }
    }
    p = add_path_element(p,buf);

    /* Set up the PHP include path before reusing rdp */
/*
    inc = apr_pstrcat(r->pool, INC_PATH, WEB_PATH, rdp, ":", SVN_PATH, rdp, NULL);
*/

    /* Extend rdp so it holds the complete path */
    rdp = apr_pstrcat(r->pool, WEB_PATH, rdp, NULL);

    /* This is the DOCUMENT_ROOT */
    conf->ap_document_root = apr_pstrcat(r->pool, rdp, NULL);

    /* Set the full path to the requested document */
    r->filename = apr_pstrcat(r->pool, rdp, r->uri, NULL);

    /* Set default PHP variables */
    /* 4 = PHP_INI_USER, 16 = PHP_INI_STAGE_RUNTIME */
/*
    zend_alter_ini_entry("safe_mode", 10, "1", strlen("1"), 4, 16);
    zend_alter_ini_entry("open_basedir", 13, inc, strlen(inc), 4, 16);
    zend_alter_ini_entry("include_path", 13, inc, strlen(inc), 4, 16);
*/
    /* Create an entry in error_log for each lookup */
    ap_log_rerror(APLOG_MARK, APLOG_ERR, 0, r, "%s", r->filename);
    return OK;
}

static void register_hooks(apr_pool_t *p)
{
    ap_hook_translate_name(rdp_translate, NULL, NULL, APR_HOOK_MIDDLE);
}

module AP_MODULE_DECLARE_DATA netserva_module =
{
    STANDARD20_MODULE_STUFF,
    NULL,                       /* dir config creater */
    NULL,                       /* dir merger --- default is to override */
    NULL,                       /* server config */
    NULL,                       /* merge server configs */
    NULL,                       /* command apr_table_t */
    register_hooks              /* register hooks */
};
