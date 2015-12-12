#!/bin/bash -x

ssh raspi mkdir -p codes/home
scp update.rb Gemfile basic.txt cron.sh ir_pattern.json raspi:codes/home
ssh raspi "cd codes/home; bundle install --path=vendor/bundle --clean"

