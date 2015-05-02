#!/bin/bash -x

scp -r template.html graph_template.html record.rb Gemfile migrations deploy_local.sh sakura2:codes/house
ssh sakura2 "cd codes/house; bash -x deploy_local.sh"

