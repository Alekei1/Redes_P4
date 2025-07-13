/*
 * Copyright 2025 NXP
 * 
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <openthread/coap.h>
#include <openthread/cli.h>
#include "LED.h"
#include "Temp_sensor.h"
#include <stdio.h>
#include <stdlib.h>

#include "Coap_Server.h"
#include <ctype.h>
#include <string.h>

#define MAX_NAME 20

otInstance *instance_g;

static char resource_name[MAX_NAME] = "Sin Nombre";

void handle_led_request(void *aContext, otMessage *aMessage, const otMessageInfo *aMessageInfo)
{
    char payload[10];
    int length = otMessageRead(aMessage, otMessageGetOffset(aMessage), payload, sizeof(payload) - 1);
    payload[length] = '\0';

    if (payload[0] == '1')
    {
        // Turn LED on
        otCliOutputFormat("Payload Recived: %s\r\n", payload);
        otCliOutputFormat("LED On \r\n");
        LED_ON();

    }
    else if (payload[0] == '0')
    {
        // Turn LED off
        otCliOutputFormat("Payload Recived: %s\r\n", payload);
        otCliOutputFormat("LED Off \r\n");
        LED_OFF();
    }

    //Send response
    otMessage *response = otCoapNewMessage(instance_g, NULL);
    otCoapMessageInitResponse(response, aMessage, OT_COAP_TYPE_ACKNOWLEDGMENT, OT_COAP_CODE_CHANGED);
    otCoapSendResponse(instance_g, response, aMessageInfo);
}

void handle_sensor_request(void *aContext, otMessage *aMessage, const otMessageInfo *aMessageInfo)
{
    static double temp_value = 0;
    temp_value = Get_Temperature();

    otMessage *response;

    if (otCoapMessageGetCode(aMessage) == OT_COAP_CODE_GET)  
    {
        response = otCoapNewMessage(instance_g, NULL);
        otCliOutputFormat("GET\r\n");

        if (response != NULL)
        {
            otCoapMessageInitResponse(response, aMessage, OT_COAP_TYPE_ACKNOWLEDGMENT, OT_COAP_CODE_CONTENT);
            
            otCoapMessageSetPayloadMarker(response);
           
            char sensorData[50] = {"0"};
            
            snprintf(sensorData, sizeof(sensorData), "%d", (int)temp_value);
            otCliOutputFormat("payload: %s\r\n", sensorData);

            otMessageAppend(response, sensorData, strlen(sensorData));

            otCoapSendResponse(instance_g, response, aMessageInfo);
        }
    }
}

void handle_name_request(void *aContext, otMessage *aMessage, const otMessageInfo *aMessageInfo)
{
	otCoapCode messageCode = otCoapMessageGetCode(aMessage);
	
	if (messageCode == OT_COAP_CODE_GET)
    {
        otMessage *response = otCoapNewMessage(instance_g, NULL);
        otCoapMessageInitResponse(response, aMessage, OT_COAP_TYPE_ACKNOWLEDGMENT, OT_COAP_CODE_CONTENT);

        otCoapMessageSetPayloadMarker(response);
        otMessageAppend(response, resource_name, strlen(resource_name));

        otCoapSendResponse(instance_g, response, aMessageInfo);
    }
    else if (messageCode == OT_COAP_CODE_PUT)
    {
        uint16_t payloadLen = otMessageGetLength(aMessage) - otMessageGetOffset(aMessage);
        if (payloadLen >= MAX_NAME) payloadLen = MAX_NAME - 1;

        otMessageRead(aMessage, otMessageGetOffset(aMessage), resource_name, payloadLen);
        resource_name[payloadLen] = '\0';

        otMessage *response = otCoapNewMessage(instance_g, NULL);
        otCoapMessageInitResponse(response, aMessage, OT_COAP_TYPE_ACKNOWLEDGMENT, OT_COAP_CODE_CHANGED);
        otCoapSendResponse(instance_g, response, aMessageInfo);
    }
    else if (messageCode == OT_COAP_CODE_DELETE)
    {
        strcpy(resource_name, "Sin Nombre");

        otMessage *response = otCoapNewMessage(instance_g, NULL);
        otCoapMessageInitResponse(response, aMessage, OT_COAP_TYPE_ACKNOWLEDGMENT, OT_COAP_CODE_DELETED);
        otCoapSendResponse(instance_g, response, aMessageInfo);
    }
 
}

void init_coap_server(otInstance *aInstance)
{
    I2C2_InitPins();
    LED_INIT();
    Temp_Sensor_start();

    instance_g = aInstance;
    
    static otCoapResource coapResource_led;
    static otCoapResource coapResource_sensor;
	static otCoapResource coapResource_name;
    
    coapResource_led.mUriPath = "led";
    coapResource_led.mHandler = handle_led_request;
    coapResource_led.mContext = NULL;
    coapResource_led.mNext = NULL;

    otCoapAddResource(aInstance, &coapResource_led);

    coapResource_sensor.mUriPath = "sensor";
    coapResource_sensor.mHandler = handle_sensor_request;
    coapResource_sensor.mContext = NULL;
    coapResource_sensor.mNext = NULL;

    otCoapAddResource(aInstance, &coapResource_sensor);
	
	coapResource_led.mUriPath = "nombre";
    coapResource_led.mHandler = handle_name_request;
    coapResource_led.mContext = NULL;
    coapResource_led.mNext = NULL;

    otCoapAddResource(aInstance, &coapResource_name);
}