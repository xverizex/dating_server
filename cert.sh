#!/usr/bin/bash
openssl req -x509 -newkey rsa:4096 -keyout temp_key.pem -out temp_cert.pem -sha256 -days 9999
openssl rsa -in temp_key.pem -out key.pem
openssl x509 -in temp_cert.pem > cert.pem
