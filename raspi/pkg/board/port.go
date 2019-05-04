package board

import (
	"github.com/tarm/serial"
)

type Port interface {
	Read(buf []byte) (int, error)
	Write(data []byte) (int, error)
	Close() error
}

type PortImpl struct {
	port *serial.Port
}

func OpenPort(config *serial.Config) (PortImpl, error) {
	port, err := serial.OpenPort(config)
	return PortImpl{
		port: port,
	}, err
}

func (p PortImpl) Read(buf []byte) (int, error) {
	return p.port.Read(buf)
}
func (p PortImpl) Write(data []byte) (int, error) {
	return p.port.Write(data)
}
func (p PortImpl) Close() error {
	return p.port.Close()
}
