package com.datetime.api;

import jakarta.persistence.*;
import java.time.LocalDateTime;

@Entity
@Table(name = "datetime_records")
public class DateTimeRecord {
    
    @Id
    @GeneratedValue(strategy = GenerationType.IDENTITY)
    private Long id;
    
    @Column(name = "begin_datetime", nullable = false)
    private LocalDateTime beginDateTime;
    
    @Column(name = "end_datetime", nullable = false)
    private LocalDateTime endDateTime;
    
    @Column(name = "created_at")
    private LocalDateTime createdAt;
    
    public DateTimeRecord() {
        this.createdAt = LocalDateTime.now();
    }
    
    public DateTimeRecord(LocalDateTime beginDateTime, LocalDateTime endDateTime) {
        this.beginDateTime = beginDateTime;
        this.endDateTime = endDateTime;
        this.createdAt = LocalDateTime.now();
    }
    
    // Getters and Setters
    public Long getId() {
        return id;
    }
    
    public void setId(Long id) {
        this.id = id;
    }
    
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
    
    public LocalDateTime getCreatedAt() {
        return createdAt;
    }
    
    public void setCreatedAt(LocalDateTime createdAt) {
        this.createdAt = createdAt;
    }
}
