package infrastructure

import (
	"encoding/json"
	"fmt"
	"io/ioutil"
	"log"

	"raspi-extboard-raspi/domain"
)

type IrDataFile struct {
	irDataMap map[string]*domain.IrData
}

func NewIrDataFile(filename string) (*IrDataFile, error) {

	txt, err := ioutil.ReadFile(filename)
	if err != nil {
		log.Printf("Error: Failed to read IR data file: %s", err.Error())
		return nil, err
	}

	var jsonData jsonIrDataMap
	err = json.Unmarshal(txt, &jsonData)
	if err != nil {
		log.Printf("Error: Failed to unmarshal IR data: %s, %s", err.Error(), txt)
		return nil, err
	}

	irDataMap := map[string]*domain.IrData{}
	for key, val := range jsonData.Ac {
		irDataMap[key] = &domain.IrData{
			Pattern:  val.Pattern,
			Interval: val.Interval,
		}
	}

	return &IrDataFile{irDataMap}, nil
}

func (i *IrDataFile) GetIrData(name string) (*domain.IrData, error) {
	res, ok := i.irDataMap[name]
	if !ok {
		log.Printf("Error: Command name not found: %s", name)
		return nil, fmt.Errorf("Error: Command name not found: %s", name)
	}
	return res, nil
}

type jsonIrDataMap struct {
	Ac map[string]struct {
		Pattern  string `json:"pattern"`
		Interval int    `json:"interval"`
	} `json:"ac"`
}
