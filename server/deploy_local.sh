#!/bin/bash -x

bundle install --path=vendor/bundle --clean
bundle exec sequel -m migrations sqlite://db.sqlite3

