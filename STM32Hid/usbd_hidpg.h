/**
  ******************************************************************************
  * @file    usbd_hid_composite.h
  * @author  MCD Application Team
  * @brief   Header file for the usbd_hid_core.c file.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2015 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                      http://www.st.com/SLA0044
  *
  ******************************************************************************
  */

#pragma once

#include "usbd_ioreq.h"

namespace hidpg
{

#define HID_MOUSE_INTERFACE 0x00U
#define HID_KEYBOARD_INTERFACE 0x01U

#define HID_MOUSE_EPIN_ADDR 0x81U
#define HID_MOUSE_EPIN_SIZE 0x04U

#define HID_KEYBOARD_EPIN_ADDR 0x82U
#define HID_KEYBOARD_EPIN_SIZE 0x08U

#define USB_COMPOSITE_HID_CONFIG_DESC_SIZ 59U
#define USB_HID_DESC_SIZ 9U
#define HID_MOUSE_REPORT_DESC_SIZE 74U
#define HID_KEYBOARD_REPORT_DESC_SIZE 45U

#define HID_DESCRIPTOR_TYPE 0x21
#define HID_REPORT_DESC 0x22

#ifndef HID_HS_BINTERVAL
#define HID_HS_BINTERVAL 0x07U
#endif

#ifndef HID_FS_BINTERVAL
#define HID_FS_BINTERVAL 0x0AU
#endif

#define HID_REQ_SET_PROTOCOL 0x0BU
#define HID_REQ_GET_PROTOCOL 0x03U

#define HID_REQ_SET_IDLE 0x0AU
#define HID_REQ_GET_IDLE 0x02U

#define HID_REQ_SET_REPORT 0x09U
#define HID_REQ_GET_REPORT 0x01U

typedef enum
{
  HID_IDLE = 0,
  HID_BUSY,
} HID_StateTypeDef;

typedef struct
{
  uint32_t Protocol;
  uint32_t IdleState;
  uint32_t AltSetting;
  HID_StateTypeDef Mousestate;
  HID_StateTypeDef Keyboardstate;
} USBD_HID_HandleTypeDef;

extern USBD_ClassTypeDef USBD_COMPOSITE_HID;
#define USBD_COMPOSITE_HID_CLASS &USBD_COMPOSITE_HID

uint8_t USBD_HID_MOUSE_SendReport(USBD_HandleTypeDef *pdev, uint8_t *report, uint16_t len);
uint8_t USBD_HID_KEYBOARD_SendReport(USBD_HandleTypeDef *pdev, uint8_t *report, uint16_t len);
uint32_t USBD_HID_GetPollingInterval(USBD_HandleTypeDef *pdev);

} // namespace hidpg
