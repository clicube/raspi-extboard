package main

import (
	"fmt"
	"os"

	"github.com/clicube/raspi-extboard/raspi/presentation"
)

func main() {
	if err := presentation.CmdlineExecute(); err != nil {
		fmt.Println(err)
		os.Exit(-1)
	}
}
