package domain

type Env struct {
	Temperature float64
	Humidity    float64
	Brightness  float64
}

type IrData struct {
	Data   string
	Period int
}
