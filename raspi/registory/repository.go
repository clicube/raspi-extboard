package registory

import (
	"raspi-extboard-raspi/domain"
	"raspi-extboard-raspi/infrastructure"
)

func EnvGetter() (domain.EnvGetter, error) {
	return &infrastructure.Board{}, nil
}

func EnvRecorder() (domain.EnvRecorder, error) {
	return &infrastructure.MultiEnvRecorder{
		Recorders: []domain.EnvRecorder{
			&infrastructure.MoshoApi{},
			&infrastructure.Datadog{},
		},
	}, nil
}
