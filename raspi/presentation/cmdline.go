package presentation

import (
	"flag"
	"fmt"

	"raspi-extboard-raspi/registory"
	"raspi-extboard-raspi/usecase"
)

func CmdlineExecute() error {
	flag.Parse()

	command := flag.Arg(0)
	switch command {
	case "envs":
		return invokeEnvs()
	case "cmd":
		return nil
	case "":
		return fmt.Errorf("Command required")
	default:
		return fmt.Errorf("Invalid command %s", command)
	}
}

func invokeEnvs() error {
	envGetter, err := registory.EnvGetter()
	if err != nil {
		return err
	}
	envRecorder, err := registory.EnvRecorder()
	if err != nil {
		return err
	}

	recEnv := usecase.RecordEnv{
		EnvGetter:   envGetter,
		EnvRecorder: envRecorder,
	}
	recEnv.Invoke()
	return nil
}
