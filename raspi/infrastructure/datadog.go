package infrastructure

import (
	"log"

	"github.com/DataDog/datadog-go/statsd"

	"raspi-extboard-raspi/domain"
)

type GaugeSender interface {
	Gauge(string, float64, []string, float64) error
}

type Datadog struct {
	client GaugeSender
}

func NewDatadog() (*Datadog, error) {
	client, err := statsd.New(
		"127.0.0.1:8125",
		statsd.WithNamespace("home."),
	)
	if err != nil {
		return nil, err
	}
	return &Datadog{client}, nil
}

func (d *Datadog) RecordEnv(env *domain.Env) error {
	log.Printf("Datadog.RecordEnv() invoked: %+v", env)
	var err error
	err = d.client.Gauge("env.temperature", env.Temperature, []string{}, 1)
	if err != nil {
		return err
	}
	err = d.client.Gauge("env.humidity", env.Humidity, []string{}, 1)
	if err != nil {
		return err
	}
	err = d.client.Gauge("env.brightness", env.Brightness, []string{}, 1)
	if err != nil {
		return err
	}
	return nil
}
