#!/bin/bash -x

ssh raspi mkdir -p codes/house
scp update.rb Gemfile salt.txt cron.sh raspi:codes/house
ssh raspi "cd codes/house; bundle install --path=vendor/bundle --clean"

