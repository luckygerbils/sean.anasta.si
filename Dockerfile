FROM alpine
MAINTAINER sean@anasta.si

VOLUME /logs
VOLUME /secrets

EXPOSE 80
EXPOSE 443

ENTRYPOINT ["/usr/local/bin/entrypoint"]

RUN apk update       \
 && apk add lighttpd curl sudo \
 && rm -r /var/cache

COPY lighttpd.conf /etc/lighttpd/lighttpd.conf
COPY entrypoint /usr/local/bin/entrypoint
COPY www /var/www/sean.anasta.si
RUN chown -R lighttpd:lighttpd /var/www/sean.anasta.si
