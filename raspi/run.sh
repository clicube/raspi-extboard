#!/bin/bash -x

ssh raspi "cd codes/home; bundle exec ruby update.rb"

