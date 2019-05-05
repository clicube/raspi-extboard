package infrastructure

import (
	"raspi-extboard-raspi/domain"
)

type MoshoApi struct{}

func (m *MoshoApi) RecordEnv(env *domain.Env) error {
	return nil
}
