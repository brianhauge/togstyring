package com.datetime.api;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.http.ResponseEntity;
import org.springframework.web.bind.annotation.*;

import java.time.LocalDateTime;
import java.util.List;

@RestController
@RequestMapping("/api/datetime")
public class DateTimeController {
    
    @Autowired
    private DateTimeRepository repository;
    
    @PostMapping
    public ResponseEntity<DateTimeRecord> setDateTime(@RequestBody DateTimeRequest request) {
        DateTimeRecord record = new DateTimeRecord(
            request.getBeginDateTime(),
            request.getEndDateTime()
        );
        DateTimeRecord saved = repository.save(record);
        return ResponseEntity.ok(saved);
    }
    
    @GetMapping("/{id}")
    public ResponseEntity<DateTimeRecord> getDateTime(@PathVariable Long id) {
        return repository.findById(id)
            .map(ResponseEntity::ok)
            .orElse(ResponseEntity.notFound().build());
    }
    
    @GetMapping
    public ResponseEntity<List<DateTimeRecord>> getAllDateTimes() {
        List<DateTimeRecord> records = repository.findAllByOrderByCreatedAtDesc();
        return ResponseEntity.ok(records);
    }
    
    @GetMapping("/latest")
    public ResponseEntity<DateTimeRecord> getLatestDateTime() {
        List<DateTimeRecord> records = repository.findAllByOrderByCreatedAtDesc();
        if (records.isEmpty()) {
            return ResponseEntity.notFound().build();
        }
        return ResponseEntity.ok(records.get(0));
    }
    
    @PutMapping("/{id}")
    public ResponseEntity<DateTimeRecord> updateDateTime(
            @PathVariable Long id,
            @RequestBody DateTimeRequest request) {
        return repository.findById(id)
            .map(record -> {
                record.setBeginDateTime(request.getBeginDateTime());
                record.setEndDateTime(request.getEndDateTime());
                DateTimeRecord updated = repository.save(record);
                return ResponseEntity.ok(updated);
            })
            .orElse(ResponseEntity.notFound().build());
    }
    
    @DeleteMapping("/{id}")
    public ResponseEntity<Void> deleteDateTime(@PathVariable Long id) {
        if (repository.existsById(id)) {
            repository.deleteById(id);
            return ResponseEntity.ok().build();
        }
        return ResponseEntity.notFound().build();
    }
}

class DateTimeRequest {
    private LocalDateTime beginDateTime;
    private LocalDateTime endDateTime;
    
    public LocalDateTime getBeginDateTime() {
        return beginDateTime;
    }
    
    public void setBeginDateTime(LocalDateTime beginDateTime) {
        this.beginDateTime = beginDateTime;
    }
    
    public LocalDateTime getEndDateTime() {
        return endDateTime;
    }
    
    public void setEndDateTime(LocalDateTime endDateTime) {
        this.endDateTime = endDateTime;
    }
}
