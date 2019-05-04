package main

import (
	"fmt"
	"os"
	"raspi-extboard-raspi/cmd/boardctl/app"
)

func main() {
	if err := app.Execute(); err != nil {
		fmt.Println(err)
		os.Exit(-1)
	}
}
