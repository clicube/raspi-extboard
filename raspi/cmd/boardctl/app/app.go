package app

import (
	"flag"
	"fmt"
	"raspi-extboard-raspi/infrastructure"
	"raspi-extboard-raspi/usecase"
)

func Execute() error {
	flag.Parse()

	command := flag.Arg(0)
	switch command {
	case "envs":
		recEnv := usecase.RecordEnv{
			EnvGetter: &infrastructure.Board{},
		}
		recEnv.Invoke()
		return nil
	case "cmd":
		return nil
	case "":
		return fmt.Errorf("Command required")
	default:
		return fmt.Errorf("Invalid command %s", command)
	}
}
