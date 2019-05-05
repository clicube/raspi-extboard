package domain

type IrSender interface {
	SendIr(IrData) error
}

type EnvGetter interface {
	GetEnv() (*Env, error)
}

type EnvRecorder interface {
	RecordEnv(*Env) error
}

type IrDataStore interface {
	GetIr() ([]IrData, error)
	DeleteIr([]IrData) error
}
