fmt:
	go fmt ./...

build:
	mkdir -p ./bin
	GOOS=linux GOARCH=arm GOARM=6 go build -o ./bin/boardctl ./cmd/boardctl

clean:
	go clean ./...
	rm -rf ./bin

test:
	go test ./...

remote-copy: build
	scp ./bin/boardctl raspi:services/raspi-extboard/raspi/

remote-envs: remote-copy
	ssh raspi "cd services/raspi-extboard/raspi && ./boardctl envs"

remote-cmd: remote-copy
	ssh raspi "cd services/raspi-extboard/raspi && ./boardctl cmd"

