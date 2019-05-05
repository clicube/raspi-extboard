package infrastructure

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

	"raspi-extboard-raspi/domain"
)

const (
	lockFilePath = "/var/tmp/boardctl.lock"
	prompt       = "RasPi-ExtBoard> "
)

var reTmp = regexp.MustCompile("TMP: ([0-9]+)")
var reHum = regexp.MustCompile("HUM: ([0-9]+)")
var reBri = regexp.MustCompile("BRI: ([0-9]+)")

type IoPort interface {
	Read(buf []byte) (int, error)
	Write(data []byte) (int, error)
	Close() error
}

type IoPortCreator interface {
	Create() (IoPort, error)
}

type DefaultPortCreator struct{}

func (*DefaultPortCreator) Create() (IoPort, error) {
	return serial.OpenPort(&serial.Config{
		Name:        "/dev/ttyAMA0",
		Baud:        9600,
		ReadTimeout: time.Millisecond * 200,
	})
}

type Board struct {
	portCreator IoPortCreator
}

func (b *Board) GetEnv() (*domain.Env, error) {
	log.Println("Getting Envs from the board ...")

	tempHumRes, err := execCommand(b.portCreator, "temp_read")
	if err != nil {
		log.Println("Error: Failed to execute: temp_read")
		return nil, err
	}

	temp, err := parseResult(tempHumRes, reTmp, "Tempreture")
	if err != nil {
		return nil, err
	}
	temp /= 10

	hum, err := parseResult(tempHumRes, reHum, "Humidity")
	if err != nil {
		return nil, err
	}
	hum /= 10

	briRes, err := execCommand(b.portCreator, "bri_read")
	if err != nil {
		log.Println("Error: Failed to execute: bri_read")
		return nil, err
	}

	bri, err := parseResult(briRes, reBri, "Brightness")
	if err != nil {
		return nil, err
	}

	return &domain.Env{
		Temperature: temp,
		Humidity:    hum,
		Brightness:  bri,
	}, nil
}

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

func execCommand(portCreatorArg IoPortCreator, cmd string) (string, error) {
	log.Printf("Executing command ... : %s", cmd)

	// Take a lock
	lock, err := lock()
	if err != nil {
		return "", err
	}
	defer unlock(lock)

	// Create a port
	var portCreator IoPortCreator
	if portCreatorArg == nil {
		portCreator = &DefaultPortCreator{}
	} else {
		portCreator = portCreatorArg
	}
	log.Println("Opening port ... ")
	port, err := portCreator.Create()
	if err != nil {
		log.Println("Error: Failed to open port")
		return "", err
	}
	defer port.Close()

	ctx := context.Background()
	ctx, cancel := context.WithTimeout(ctx, time.Second*5)
	defer cancel()
	ch := make(chan string, 1)

	go func() {
		log.Println("Writing to port ... ")
		_, err = port.Write([]byte(cmd + "\n"))
		if err != nil {
			log.Println("Error: Failed to write to port")
			cancel()
		}

		log.Println("Reading from port ... ")
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

func parseResult(data string, re *regexp.Regexp, name string) (float64, error) {
	matched := re.FindStringSubmatch(data)
	if matched == nil {
		log.Printf("Error: Failed to get %s", name)
		return 0, fmt.Errorf("Failed to get %s", name)
	}
	val, err := strconv.ParseFloat(matched[1], 64)
	if err != nil {
		log.Printf("Error: Failed to parse %s", name)
		return 0, err
	}
	log.Printf("%s: %f", name, val)

	return val, nil
}