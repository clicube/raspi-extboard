package infrastructure

import (
	"bytes"
	"encoding/json"
	"fmt"
	"log"
	"net/http"
	"time"
	"io/ioutil"
	"strings"

	"raspi-extboard-raspi/domain"
)

type MoshoApi struct {
	client *http.Client
	basicUser string
	basicPass string
}

const (
	baseUrl = "https://asia-northeast1-mosho-166cd.cloudfunctions.net"
)

func NewMoshoApi() (*MoshoApi, error) {
	client := http.DefaultClient

		txtbyte,err := ioutil.ReadFile("basic.txt")
	if err != nil {
		log.Printf("Error: Failed to read credential file: %+v", err)
		return nil, err
	}
	txt := strings.TrimRight(string(txtbyte), "\n")
	cred := strings.Split(string(txt), ":")

	return &MoshoApi{
		client: client,
		basicUser: cred[0],
		basicPass: cred[1],
		}, nil
}

func (m *MoshoApi) RecordEnv(env *domain.Env) error {
	log.Printf("MoshoApi.RecordEnv() invoked: %+v", env)

	time := time.Now().Unix()
	postEntity := postEnv{
		Temperature: env.Temperature,
		Humidity:    env.Humidity,
		Brightness:  env.Brightness,
		Time:        time,
	}

	postJson, err := json.Marshal(postEntity)
	if err != nil {
		log.Printf("Error: Failed to marshal JSON: %+v, %+v", err, postEntity)
		return err
	}

	req, err := http.NewRequest("POST", baseUrl+"/api/v1/envs", bytes.NewBuffer(postJson))
	if err != nil {
		log.Printf("Error: Failed to create Request: %+v", err)
		return err
	}

	req.SetBasicAuth(m.basicUser, m.basicPass)
	req.Header.Set("Content-Type", "application/json")

	res, err := m.client.Do(req)
	if err != nil {
		log.Printf("Request : %+v", req)
		log.Printf("Error: Failed to do request: %+v, %+v", err, req)
		return err
	}
	if res.StatusCode != 200 {
		log.Printf("Request : %+v", req)
		log.Printf("Response: %+v", res)
		log.Printf("Error: Status: %s", res.Status)
		return fmt.Errorf("Error: Mosho API Status: %s", res.Status)
	}
	res.Body.Close()
	log.Printf("Status: %s", res.Status)

	return nil
}

type postEnv struct {
	Temperature float64 `json:"temperature"`
	Humidity    float64 `json:"humidity"`
	Brightness  float64 `json:"brightness"`
	Time        int64   `json:"time"`
}
