#!/bin/bash -x

ssh raspi "cd codes/house; bundle exec ruby update.rb"

