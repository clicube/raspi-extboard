package app

import (
	"flag"
	"fmt"
	"raspi-extboard-raspi/pkg/board"
	// "raspi-extboard-raspi/pkg/moshoapi"
)

func Execute() error {
	flag.Parse()

	command := flag.Arg(0)
	switch command {
	case "envs":
		_, err := board.GetEnvs(nil)
		if err != nil {
			return err
		}
		// datadog.Upload(envs)
		// moshoapi.Upload(envs)

		return nil
	case "cmd":
		//board.GetEnvs()
		return nil
	case "":
		return fmt.Errorf("Command required")
	default:
		return fmt.Errorf("Invalid command %s", command)
	}
}
