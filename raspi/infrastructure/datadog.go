package infrastructure

import (
	"raspi-extboard-raspi/domain"
)

type DataDog struct{}

func (d *DataDog) RecordEnv(env *domain.Env) error {
	return nil
}
