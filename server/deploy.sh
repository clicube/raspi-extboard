#!/bin/bash -x

scp -r template.html record.rb Gemfile migrations deploy_local.sh cubik:codes/house
ssh cubik "cd codes/house; bash -x deploy_local.sh"

