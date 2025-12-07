package com.mqtt.bridge;

import com.fasterxml.jackson.databind.ObjectMapper;
import com.fasterxml.jackson.datatype.jsr310.JavaTimeModule;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.beans.factory.annotation.Value;
import org.springframework.messaging.Message;
import org.springframework.messaging.MessageHandler;
import org.springframework.messaging.MessagingException;
import org.springframework.stereotype.Component;
import org.springframework.web.client.RestTemplate;

@Component
public class MqttMessageHandler implements MessageHandler {
    
    private static final Logger logger = LoggerFactory.getLogger(MqttMessageHandler.class);
    
    @Value("${api.base.url}")
    private String apiBaseUrl;
    
    private final RestTemplate restTemplate;
    private final ObjectMapper objectMapper;
    
    public MqttMessageHandler() {
        this.restTemplate = new RestTemplate();
        this.objectMapper = new ObjectMapper();
        this.objectMapper.registerModule(new JavaTimeModule());
    }
    
    @Override
    public void handleMessage(Message<?> message) throws MessagingException {
        try {
            String payload = message.getPayload().toString();
            logger.info("Received MQTT message: {}", payload);
            
            // Parse the JSON payload
            TrainDetectionDto detection = objectMapper.readValue(payload, TrainDetectionDto.class);
            logger.info("Parsed train detection: {}", detection);
            
            // Post to API
            String apiUrl = apiBaseUrl + "/api/train/detection";
            TrainDetectionDto response = restTemplate.postForObject(apiUrl, detection, TrainDetectionDto.class);
            
            logger.info("Successfully posted to API: {}", response);
            
        } catch (Exception e) {
            logger.error("Error processing MQTT message", e);
            throw new MessagingException("Failed to process MQTT message", e);
        }
    }
}
