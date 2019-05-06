package domain

type EnvGetter interface {
	GetEnv() (*Env, error)
}

type EnvRecorder interface {
	RecordEnv(*Env) error
}

type IrCommandRepository interface {
	GetIrCommands() ([]*IrCommand, error)
	DeleteIrCommand(*IrCommand) error
}

type IrDataRepository interface {
	GetIrData(string) (*IrData, error)
}

type IrDataSender interface {
	SendIr(*IrData) error
}
