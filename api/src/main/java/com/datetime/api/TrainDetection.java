package com.datetime.api;

import jakarta.persistence.*;
import java.time.LocalDateTime;

@Entity
@Table(name = "train_detections")
public class TrainDetection {
    
    @Id
    @GeneratedValue(strategy = GenerationType.IDENTITY)
    private Long id;
    
    @Column(name = "state", nullable = false)
    private String state;
    
    @Column(name = "rounds", nullable = false)
    private Integer rounds;
    
    @Column(name = "relay", nullable = false)
    private String relay;
    
    @Column(name = "timestamp", nullable = false)
    private LocalDateTime timestamp;
    
    @Column(name = "created_at")
    private LocalDateTime createdAt;
    
    public TrainDetection() {
        this.createdAt = LocalDateTime.now();
    }
    
    public TrainDetection(String state, Integer rounds, String relay, LocalDateTime timestamp) {
        this.state = state;
        this.rounds = rounds;
        this.relay = relay;
        this.timestamp = timestamp;
        this.createdAt = LocalDateTime.now();
    }
    
    // Getters and Setters
    public Long getId() {
        return id;
    }
    
    public void setId(Long id) {
        this.id = id;
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
    
    public LocalDateTime getCreatedAt() {
        return createdAt;
    }
    
    public void setCreatedAt(LocalDateTime createdAt) {
        this.createdAt = createdAt;
    }
}
