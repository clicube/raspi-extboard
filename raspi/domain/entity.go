package domain

type Env struct {
	Temperature float64
	Humidity    float64
	Brightness  float64
}

type IrData struct {
	Pattern  string
	Interval int
}

type IrCommand struct {
	Id      int64
	Command string
}
