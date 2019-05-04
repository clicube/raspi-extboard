package moshoapi

import (
	"raspi-extboard-raspi/pkg/entity"
)

type MoshoApi interface {
	Upload(*entity.Envs) error
}

