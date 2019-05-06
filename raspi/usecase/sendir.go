package usecase

import (
	"log"

	"raspi-extboard-raspi/domain"
)

type SendIr struct {
	IrCommandRepository domain.IrCommandRepository
	IrDataRepository    domain.IrDataRepository
	IrDataSender        domain.IrDataSender
}

func (u *SendIr) Invoke() error {
	log.Println("usecase.SendIr invoked")

	cmds, err := u.IrCommandRepository.GetIrCommands()
	if err != nil {
		log.Println("Error: Failed to get commands")
		return err
	}

	for _, cmd := range cmds {
		log.Printf("Command: %+v", cmd)

		name := cmd.Command

		data, err := u.IrDataRepository.GetIrData(name)
		if err != nil {
			log.Printf("Error: Failed to get IR Data for %s", name)
		} else {

			u.IrDataSender.SendIr(data)
			if err != nil {
				log.Printf("Error: Failed to send IR Data for %s", name)
			}

		}

		err = u.IrCommandRepository.DeleteIrCommand(cmd)
		if err != nil {
			log.Printf("Error: Failed to delete IR command id=%d", cmd.Id)
		}
	}

	return err
}
