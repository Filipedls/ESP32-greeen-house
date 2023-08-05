FROM php:8.1-apache

RUN apt-get update && apt-get upgrade -y

RUN docker-php-ext-install mysqli && docker-php-ext-enable mysqli

# RUN apt-get install sudo unzip wget -y
 
# RUN a2enmod rewrite
# RUN a2enmod ssl
# RUN service apache2 restart
 
EXPOSE 80