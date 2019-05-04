package board

import (
	"bytes"
	"context"
	"fmt"
	"log"
	"regexp"
	"strconv"
	"strings"
	"time"

	"github.com/gofrs/flock"
	"github.com/tarm/serial"

	"raspi-extboard-raspi/pkg/entity"
)

const (
	lockFilePath = "/var/tmp/boardctl.lock"
	prompt       = "RasPi-ExtBoard> "
)

var reTmp = regexp.MustCompile("TMP: ([0-9]+)")
var reHum = regexp.MustCompile("HUM: ([0-9]+)")
var reBri = regexp.MustCompile("BRI: ([0-9]+)")

func lock() (*flock.Flock, error) {
	fileLock := flock.New(lockFilePath)
	err := fileLock.Lock()
	if err != nil {
		log.Printf("Error: Failed to lock the lock file: %s", lockFilePath)
		return nil, err
	}
	log.Printf("Locked lock file: %s", lockFilePath)
	return fileLock, nil
}

func unlock(fileLock *flock.Flock) error {
	log.Printf("Unlocked lock file: %s", lockFilePath)
	return fileLock.Unlock()
}

func openDefaultPort() (PortImpl, error) {
	return OpenPort(&serial.Config{
		Name:        "/dev/ttyAMA0",
		Baud:        9600,
		ReadTimeout: time.Millisecond * 200,
	})
}

func execCommand(givenPort Port, cmd string) (string, error) {
	log.Printf("Executing command ... : %s", cmd)

	// Take a lock
	lock, err := lock()
	if err != nil {
		return "", err
	}
	defer unlock(lock)

	// Open a port if not given
	var port Port
	if givenPort == nil {
		port, err = openDefaultPort()
		if err != nil {
			log.Println("Error: Failed to open selial port")
			return "", err
		}
	} else {
		port = givenPort
	}
	defer port.Close()

	ctx := context.Background()
	ctx, cancel := context.WithTimeout(ctx, time.Second*5)
	defer cancel()
	ch := make(chan string, 1)

	go func() {
		log.Println("Writing to serialport ... ")
		_, err = port.Write([]byte(cmd + "\n"))
		if err != nil {
			log.Println("Error: Failed to write to serialport")
			cancel()
		}

		log.Println("Reading from serialport ... ")
		buf := bytes.Buffer{}

		for {
			rbuf := make([]byte, 128)
			n, err := port.Read(rbuf)
			if err != nil {
				// log.Println("Read: No data")
			} else {
				// log.Println("Read: " + string(rbuf) + "")
				buf.Write(rbuf[0:n])

				// Check if command is complete
				bufstr := buf.String()
				if strings.HasSuffix(bufstr, prompt) {
					log.Printf("Data: %s", buf.String())
					ch <- bufstr
					break
				}
			}
		}
	}()
	select {
	case res := <-ch:
		lines := strings.Split(res, "\n")
		filteredLines := lines[1 : len(lines)-1]
		return strings.Join(filteredLines, "\n"), nil
	case <-ctx.Done():
		log.Println("Error: Command timeout.")
		return "", fmt.Errorf("Command timeout: %s", cmd)
	}
}

func GetEnvs(port Port) (*entity.Envs, error) {
	log.Println("Getting Envs from the board ...")

	tempHumRes, err := execCommand(port, "temp_read")
	if err != nil {
		log.Println("Error: Failed to execute: temp_read")
		return nil, err
	}

	tempMatched := reTmp.FindStringSubmatch(tempHumRes)
	if tempMatched == nil {
		log.Println("Error: Failed to get temperature")
		return nil, fmt.Errorf("Failed to get temperature")
	}
	temp, err := strconv.ParseFloat(tempMatched[1], 64)
	if err != nil {
		log.Println("Error: Failed to parse temperature")
		return nil, err
	}
	temp = temp / 10
	log.Printf("Temperature: %f", temp)

	humMatched := reHum.FindStringSubmatch(tempHumRes)
	if humMatched == nil {
		log.Println("Error: Failed to get humidity")
		return nil, fmt.Errorf("Failed to get humidity")
	}
	hum, err := strconv.ParseFloat(humMatched[1], 64)
	if err != nil {
		log.Println("Error: Failed to parse humidity")
		return nil, err
	}
	hum = hum / 10
	log.Printf("Humidity: %f", hum)

	briRes, err := execCommand(port, "bri_read")
	if err != nil {
		log.Println("Error: Failed to execute: bri_read")
		return nil, err
	}

	briMatched := reBri.FindStringSubmatch(briRes)
	if briMatched == nil {
		log.Println("Error: Failed to get brightness")
		return nil, fmt.Errorf("Failed to get brightness")
	}
	bri, err := strconv.ParseFloat(briMatched[1], 64)
	if err != nil {
		log.Println("Error: Failed to parse brightness")
		return nil, err
	}
	log.Printf("Brightness: %f", bri)

	return &entity.Envs{
		Temperature: temp,
		Humidity:    hum,
		Brightness:  bri,
	}, nil
}
