FROM centos

MAINTAINER Nyx Project <chenpeiwen6@gmial.com>

RUN yum -y install libev-devel
RUN yum -y install boost-devel
RUN yum -y install gcc automake autoconf libtool make
RUN yum -y install gcc-c++
RUN yum -y install libcurl-devel
RUN yum -y install sqlite-devel
RUN yum -y install zlib-devel
RUN yum -y install openssl-devel
