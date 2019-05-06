package infrastructure

import (
	"log"

	"raspi-extboard-raspi/domain"
)

type MoshoApi struct{}

func (m *MoshoApi) RecordEnv(env *domain.Env) error {
	log.Printf("MoshoApi.RecordEnv() invoked: %+v", env)
	return nil
}
