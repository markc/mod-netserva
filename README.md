# mod-netserva

## Reverse Domain Path Apache2 Module

An Apache module to dynamically transform incoming requests for a virtual
host to a reversed domainname pathed (RDP) DocumentRoot. For example, a
domain of **some.example.com** gets transformed to **com/example/some**
and appended to whatever WEB_PATH is set to;

    http://some.example.com -> WEB_PATH/com/example/some

If wildcard DNS entries are set up for vhosted domains then nothing more
than the existance of a directory is needed for a vhost to resolve, ie;
no VirtualHost entries are required in the Apache configuration files.

For now, WEB_PATH is hardwired and defined in the program itself. It
should be a configuration option. The apache development headers and
**apxs2** program needs to be available. These come with the Archlinux
apache package but Debian needs the apache2-dev package.

The auto PHP safe_mode settings have been disabled until the problem
with missing SUHOSIN headers is worked out.

For extra debugging these can be added towards the end of the file.

    fprintf(stderr,"r->hostname = %s\n", r->hostname);
    fprintf(stderr,"r->the_request = %s\n", r->the_request);
    fprintf(stderr,"r->protocol = %s\n", r->protocol);
    fprintf(stderr,"r->uri = %s\n", r->uri);
    fprintf(stderr,"r->filename = %s\n\n", r->filename);
    fflush(stderr);

* Source: http://github.com/markc/mod-netserva
* Version: v0.0.6 20110715
* Copyright: (C) 2011-2017 Mark Constable <markc@renta.net>
* License: AGPL-3.0 - http://www.gnu.org/licenses/agpl-3.0.txt
