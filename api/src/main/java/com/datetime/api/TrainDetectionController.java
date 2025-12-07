package com.datetime.api;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.http.ResponseEntity;
import org.springframework.web.bind.annotation.*;

import java.time.LocalDateTime;
import java.util.List;
import java.util.stream.Collectors;

@RestController
@RequestMapping("/api/train")
public class TrainDetectionController {
    
    @Autowired
    private TrainDetectionRepository repository;
    
    // ESP32 format POST endpoint
    @PostMapping("/detection")
    public ResponseEntity<Esp32Response> postDetection(@RequestBody Esp32Request request) {
        TrainDetection detection = new TrainDetection(
            request.getState(),
            request.getRounds(),
            request.getRelay(),
            request.getTimestamp()
        );
        TrainDetection saved = repository.save(detection);
        
        Esp32Response response = new Esp32Response(
            saved.getState(),
            saved.getRounds(),
            saved.getRelay(),
            saved.getTimestamp()
        );
        return ResponseEntity.ok(response);
    }
    
    // ESP32 format GET endpoint - returns latest detection
    @GetMapping("/detection/latest")
    public ResponseEntity<Esp32Response> getLatestDetection() {
        List<TrainDetection> detections = repository.findTop10ByOrderByTimestampDesc();
        if (detections.isEmpty()) {
            return ResponseEntity.notFound().build();
        }
        
        TrainDetection latest = detections.get(0);
        Esp32Response response = new Esp32Response(
            latest.getState(),
            latest.getRounds(),
            latest.getRelay(),
            latest.getTimestamp()
        );
        return ResponseEntity.ok(response);
    }
    
    // ESP32 format GET endpoint - returns all detections
    @GetMapping("/detection")
    public ResponseEntity<List<Esp32Response>> getAllDetections() {
        List<TrainDetection> detections = repository.findAllByOrderByTimestampDesc();
        
        List<Esp32Response> responses = detections.stream()
            .map(d -> new Esp32Response(d.getState(), d.getRounds(), d.getRelay(), d.getTimestamp()))
            .collect(Collectors.toList());
        
        return ResponseEntity.ok(responses);
    }
    
    // Get detection by ID
    @GetMapping("/detection/{id}")
    public ResponseEntity<Esp32Response> getDetectionById(@PathVariable Long id) {
        return repository.findById(id)
            .map(d -> ResponseEntity.ok(new Esp32Response(
                d.getState(), 
                d.getRounds(), 
                d.getRelay(), 
                d.getTimestamp()
            )))
            .orElse(ResponseEntity.notFound().build());
    }
    
    // Get statistics
    @GetMapping("/stats")
    public ResponseEntity<TrainStats> getStats() {
        List<TrainDetection> all = repository.findAll();
        
        long totalDetections = all.size();
        long activatedCount = all.stream()
            .filter(d -> "activated".equals(d.getRelay()))
            .count();
        long notActivatedCount = totalDetections - activatedCount;
        
        Integer maxRounds = all.stream()
            .map(TrainDetection::getRounds)
            .max(Integer::compareTo)
            .orElse(0);
        
        TrainStats stats = new TrainStats(
            totalDetections,
            activatedCount,
            notActivatedCount,
            maxRounds
        );
        
        return ResponseEntity.ok(stats);
    }
}

// Request/Response classes matching ESP32 format
class Esp32Request {
    private String state;
    private Integer rounds;
    private String relay;
    private LocalDateTime timestamp;
    
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
}

class Esp32Response {
    private String state;
    private Integer rounds;
    private String relay;
    private LocalDateTime timestamp;
    
    public Esp32Response(String state, Integer rounds, String relay, LocalDateTime timestamp) {
        this.state = state;
        this.rounds = rounds;
        this.relay = relay;
        this.timestamp = timestamp;
    }
    
    public String getState() {
        return state;
    }
    
    public Integer getRounds() {
        return rounds;
    }
    
    public String getRelay() {
        return relay;
    }
    
    public LocalDateTime getTimestamp() {
        return timestamp;
    }
}

class TrainStats {
    private Long totalDetections;
    private Long activatedCount;
    private Long notActivatedCount;
    private Integer maxRounds;
    
    public TrainStats(Long totalDetections, Long activatedCount, Long notActivatedCount, Integer maxRounds) {
        this.totalDetections = totalDetections;
        this.activatedCount = activatedCount;
        this.notActivatedCount = notActivatedCount;
        this.maxRounds = maxRounds;
    }
    
    public Long getTotalDetections() {
        return totalDetections;
    }
    
    public Long getActivatedCount() {
        return activatedCount;
    }
    
    public Long getNotActivatedCount() {
        return notActivatedCount;
    }
    
    public Integer getMaxRounds() {
        return maxRounds;
    }
}
