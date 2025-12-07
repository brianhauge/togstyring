package com.datetime.api;

import org.springframework.data.jpa.repository.JpaRepository;
import org.springframework.stereotype.Repository;

import java.util.List;

@Repository
public interface TrainDetectionRepository extends JpaRepository<TrainDetection, Long> {
    List<TrainDetection> findAllByOrderByTimestampDesc();
    List<TrainDetection> findTop10ByOrderByTimestampDesc();
}
