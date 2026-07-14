# Week 1 - Day 4 (Friday): Multi-Channel ADC Polling

Polls 5 ADC channels simultaneously, each with oversampling + median filtering.

## What it does
- Reads all 5 sensor channels in a loop
- Applies 16x oversampling + median filter to each
- Logs raw vs filtered values for noise floor verification
