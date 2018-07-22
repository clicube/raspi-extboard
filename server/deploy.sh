#!/bin/bash -x

scp -r Dockerfile app sakura1:docker/compose/services/home
ssh sakura1 "cd docker/compose/services/home && 
    docker-compose build home &&
    docker-compose stop home &&
    docker-compose rm -f home &&
    docker-compose up -d home &&
    docker-compose stop nginx &&
    docker-compose rm -f nginx &&
    docker-compose up -d nginx &&
    docker-compose ps"
