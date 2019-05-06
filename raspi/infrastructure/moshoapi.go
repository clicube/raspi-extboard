package infrastructure

import (
	"bytes"
	"encoding/json"
	"fmt"
	"io/ioutil"
	"log"
	"net/http"
	"strconv"
	"strings"
	"time"

	"raspi-extboard-raspi/domain"
)

type MoshoApi struct {
	client    *http.Client
	basicUser string
	basicPass string
}

const (
	baseUrl = "https://asia-northeast1-mosho-166cd.cloudfunctions.net"
)

func NewMoshoApi() (*MoshoApi, error) {
	client := http.DefaultClient

	txtbyte, err := ioutil.ReadFile("basic.txt")
	if err != nil {
		log.Printf("Error: Failed to read credential file: %+v", err)
		return nil, err
	}
	txt := strings.TrimRight(string(txtbyte), "\n")
	cred := strings.Split(string(txt), ":")

	return &MoshoApi{
		client:    client,
		basicUser: cred[0],
		basicPass: cred[1],
	}, nil
}

func (m *MoshoApi) RecordEnv(env *domain.Env) error {
	log.Printf("MoshoApi.RecordEnv() invoked: %+v", env)

	time := time.Now().Unix()
	postEntity := jsonEnv{
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
	defer res.Body.Close()
	if res.StatusCode != 200 {
		log.Printf("Request : %+v", req)
		log.Printf("Response: %+v", res)
		log.Printf("Error: Status: %s", res.Status)
		return fmt.Errorf("Error: Mosho API Status: %s", res.Status)
	}
	log.Printf("Status: %s", res.Status)

	return nil
}

func (m *MoshoApi) GetIrCommands() ([]*domain.IrCommand, error) {
	log.Println("MoshoApi.GetIrCommands() invoked")

	req, err := http.NewRequest("GET", baseUrl+"/api/v1/ac/commands", nil)
	if err != nil {
		log.Printf("Error: Failed to create Request: %+v", err)
		return nil, err
	}

	req.SetBasicAuth(m.basicUser, m.basicPass)
	req.Header.Set("Content-Type", "application/json")

	log.Println("Getting IR Commands ...")
	res, err := m.client.Do(req)
	if err != nil {
		log.Printf("Request : %+v", req)
		log.Printf("Error: Failed to do request: %+v, %+v", err, req)
		return nil, err
	}
	defer res.Body.Close()
	if res.StatusCode != 200 {
		log.Printf("Request : %+v", req)
		log.Printf("Response: %+v", res)
		log.Printf("Error: Status: %s", res.Status)
		return nil, fmt.Errorf("Error: Mosho API Status: %s", res.Status)
	}
	log.Printf("Status: %s", res.Status)

	log.Println("Parsing IR Commands ...")
	var jsoncmds []jsonIrCommand
	body, err := ioutil.ReadAll(res.Body)
	if err != nil {
		log.Printf("Error: Failed to read body: %+v", err)
		return nil, err
	}

	err = json.Unmarshal([]byte(body), &jsoncmds)
	if err != nil {
		log.Printf("Error: Failed to unmarshal body: %+v, %s", err, body)
		return nil, err
	}

	cmds := []*domain.IrCommand{}
	for _, jsoncmd := range jsoncmds {
		cmd := &domain.IrCommand{
			Id:      jsoncmd.Id,
			Command: jsoncmd.Command,
		}
		cmds = append(cmds, cmd)
	}

	log.Printf("%d command(s)", len(cmds))

	return cmds, nil
}

func (m *MoshoApi) DeleteIrCommand(cmd *domain.IrCommand) error {
	log.Println("MoshoApi.DeleteIrCommand() invoked")

	url := baseUrl + "/api/v1/ac/commands/" + strconv.FormatInt(cmd.Id, 10)
	req, err := http.NewRequest("DELETE", url, nil)
	if err != nil {
		log.Printf("Error: Failed to create Request: %+v", err)
		return err
	}

	req.SetBasicAuth(m.basicUser, m.basicPass)
	req.Header.Set("Content-Type", "application/json")

	log.Println("Deleting IR Command ...")
	res, err := m.client.Do(req)
	if err != nil {
		log.Printf("Request : %+v", req)
		log.Printf("Error: Failed to do request: %+v, %+v", err, req)
		return err
	}
	defer res.Body.Close()
	if res.StatusCode != 200 {
		log.Printf("Request : %+v", req)
		log.Printf("Response: %+v", res)
		log.Printf("Error: Status: %s", res.Status)
		return fmt.Errorf("Error: Mosho API Status: %s", res.Status)
	}
	log.Printf("Status: %s", res.Status)

	return nil
}

type jsonEnv struct {
	Temperature float64 `json:"temperature"`
	Humidity    float64 `json:"humidity"`
	Brightness  float64 `json:"brightness"`
	Time        int64   `json:"time"`
}

type jsonIrCommand struct {
	Id      int64  `json:"id"`
	Command string `json:"command"`
}
