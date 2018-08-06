#!/bin/bash -x
cd `dirname $0`

if [ -n "${1}" ]; then
    mode=${1}
elif [ -n "${MODE}" ]; then
    mode=${MODE}
else
    mode=development
fi

cd web
rm dist/bundle.js
npx webpack --mode=${mode}
cd ..

scp -r Dockerfile app sakura1:docker/compose/services/home

ssh sakura1 "rm -rf docker/compose/services/home/app/public/ng"
scp -r web/dist sakura1:docker/compose/services/home/app/public/ng

ssh sakura1 "cd docker/compose/services/home &&
    docker-compose build home &&
    docker-compose stop home &&
    docker-compose rm -f home &&
    docker-compose up -d home &&
    docker-compose stop nginx &&
    docker-compose rm -f nginx &&
    docker-compose up -d nginx &&
    docker-compose ps"
