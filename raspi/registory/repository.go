package registory

import (
	"raspi-extboard-raspi/domain"
	"raspi-extboard-raspi/infrastructure"
)

func EnvGetter() (domain.EnvGetter, error) {
	return &infrastructure.Board{}, nil
}

func EnvRecorder() (domain.EnvRecorder, error) {
	datadog, err := infrastructure.NewDatadog()
	if err != nil {
		return nil, err
	}
	moshoapi, err := infrastructure.NewMoshoApi()
	if err != nil {
		return nil, err
	}
	return &infrastructure.MultiEnvRecorder{
		Recorders: []domain.EnvRecorder{datadog, moshoapi},
	}, nil
}

func IrCommandRepository() (domain.IrCommandRepository, error) {
	return infrastructure.NewMoshoApi()
}

func IrDataRepository() (domain.IrDataRepository, error) {
	return infrastructure.NewIrDataFile("ir_pattern.json")
}

func IrDataSender() (domain.IrDataSender, error) {
	return &infrastructure.Board{}, nil
}
