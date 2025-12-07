package com.mqtt.bridge;

import com.fasterxml.jackson.annotation.JsonProperty;
import java.time.LocalDateTime;

public class TrainDetectionDto {
    
    @JsonProperty("state")
    private String state;
    
    @JsonProperty("rounds")
    private Integer rounds;
    
    @JsonProperty("relay")
    private String relay;
    
    @JsonProperty("timestamp")
    private LocalDateTime timestamp;
    
    public TrainDetectionDto() {
    }
    
    public TrainDetectionDto(String state, Integer rounds, String relay, LocalDateTime timestamp) {
        this.state = state;
        this.rounds = rounds;
        this.relay = relay;
        this.timestamp = timestamp;
    }
    
    public String getState() {
        return state;
    }
    
    public void setState(String state) {
        this.state = state;
    }
    
    public Integer getRounds() {
        return rounds;
    }
    
    public void setRounds(Integer rounds) {
        this.rounds = rounds;
    }
    
    public String getRelay() {
        return relay;
    }
    
    public void setRelay(String relay) {
        this.relay = relay;
    }
    
    public LocalDateTime getTimestamp() {
        return timestamp;
    }
    
    public void setTimestamp(LocalDateTime timestamp) {
        this.timestamp = timestamp;
    }
    
    @Override
    public String toString() {
        return "TrainDetectionDto{" +
                "state='" + state + '\'' +
                ", rounds=" + rounds +
                ", relay='" + relay + '\'' +
                ", timestamp=" + timestamp +
                '}';
    }
}
