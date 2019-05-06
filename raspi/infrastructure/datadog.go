package infrastructure

import (
	"log"

	"raspi-extboard-raspi/domain"
)

type Datadog struct{}

func (d *Datadog) RecordEnv(env *domain.Env) error {
	log.Printf("Datadog.RecordEnv() invoked: %+v", env)
	return nil
}
