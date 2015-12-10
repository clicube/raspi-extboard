#!/bin/bash -x

scp -r Dockerfile app sakura1:docker/house
ssh sakura1 "cd docker && 
    docker-compose build house &&
    docker-compose stop house &&
    docker-compose rm -f house &&
    docker-compose up -d house &&
    docker-compose stop nginx &&
    docker-compose rm -f nginx &&
    docker-compose up -d nginx &&
    docker-compose ps"
