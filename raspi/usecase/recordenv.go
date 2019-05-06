package usecase

import (
	"log"

	"raspi-extboard-raspi/domain"
)

type RecordEnv struct {
	EnvGetter   domain.EnvGetter
	EnvRecorder domain.EnvRecorder
}

func (u *RecordEnv) Invoke() error {
	log.Println("usecase.RecordEnv invoked")
	env, err := u.EnvGetter.GetEnv()
	if err != nil {
		log.Println("Error: Failed to get env")
		return err
	}
	err = u.EnvRecorder.RecordEnv(env)
	return err
}
