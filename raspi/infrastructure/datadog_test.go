package infrastructure

import (
	"testing"

	"raspi-extboard-raspi/domain"
)

type MockDatadogClient struct{}

func (m *MockDatadogClient) Gauge(string, float64, []string, float64) error {
	return nil
}
func TestDatadogRecordEnv(t *testing.T) {
	// Arrange
	client := &MockDatadogClient{}
	sut := Datadog{client}
	env := &domain.Env{
		Temperature: 1,
		Humidity:    2,
		Brightness:  3,
	}

	// Act
	err := sut.RecordEnv(env)

	// Assert
	if err != nil {
		t.Fatalf("Datadog.RecordEnv returns error: %v", err)
	}
}
