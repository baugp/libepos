/***************************************************************************
 *   Copyright (C) 2008 by Ralf Kaestner                                   *
 *   ralf.kaestner@gmail.com                                               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include <stdio.h>
#include <string.h>

#include "device.h"

const char* epos_device_errors[] = {
  "success",
  "error initializing EPOS device",
  "error closing EPOS device",
  "invalid EPOS object size",
  "error sending to EPOS device",
  "error receiving from EPOS device",
  "EPOS communication error (abort error)",
  "EPOS internal device error",
  "error reading from EPOS device",
  "error writing to EPOS device",
  "invalid CAN bitrate",
  "invalid RS232 baudrate",
};

int epos_device_can_bitrates[] = {
  1000,
  800,
  500,
  250,
  125,
  50,
  20,
};

int epos_device_rs232_baudrates[] = {
  9600,
  14400,
  19200,
  38400,
  57600,
  115200,
};

int epos_device_init(epos_device_p dev, int node_id, can_parameter_t
  parameters[], ssize_t num_parameters) {
  if (!can_init(&dev->can_dev, parameters, num_parameters)) {
    dev->num_read = 0;
    dev->num_written = 0;

    dev->node_id = epos_device_get_id(dev);

    dev->can_bitrate = epos_device_get_can_bitrate(dev);
    dev->rs232_baudrate = epos_device_get_rs232_baudrate(dev);

    dev->hardware_version = epos_device_get_hardware_version(dev);
    dev->software_version = epos_device_get_software_version(dev);

    return epos_device_reset(dev);
  }
  else {
    fprintf(stderr, "Node %d connection error\n", node_id);
    return EPOS_DEVICE_ERROR_INIT;
  }
}

int epos_device_close(epos_device_p dev) {
  if (!can_close(&dev->can_dev)) {
    dev->node_id = EPOS_DEVICE_INVALID_ID;
    return EPOS_DEVICE_ERROR_NONE;
  }
  else
    return EPOS_DEVICE_ERROR_CLOSE;
}

int epos_device_send_message(epos_device_p dev, can_message_p message) {
  if (!can_send_message(&dev->can_dev, message))
    return EPOS_DEVICE_ERROR_NONE;
  else
    return EPOS_DEVICE_ERROR_SEND;
}

int epos_device_receive_message(epos_device_p dev, can_message_p message) {
  if (!can_receive_message(&dev->can_dev, message)) {
    if ((message->id >= EPOS_DEVICE_EMERGENCY_ID) &&
      (message->id <= EPOS_DEVICE_EMERGENCY_ID+EPOS_DEVICE_MAX_ID)) {
      short code;

      memcpy(&code, &message->content[0], sizeof(code));
      fprintf(stderr, "Node %d device error: %s\n",
        message->id-EPOS_DEVICE_EMERGENCY_ID,
        epos_error_get_device(code));

      return EPOS_DEVICE_ERROR_INTERNAL;
    }
    else if (message->content[0] == EPOS_DEVICE_ABORT) {
      int code;

      memcpy(&code, &message->content[4], sizeof(code));
      fprintf(stderr, "Node %d communication error: %s\n",
        message->id-EPOS_DEVICE_RECEIVE_ID,
        epos_error_get_comm(code));

      return EPOS_DEVICE_ERROR_ABORT;
    }
    else
      return EPOS_DEVICE_ERROR_NONE;
  }
  else
    return EPOS_DEVICE_ERROR_RECEIVE;
}

int epos_device_read(epos_device_p dev, short index, unsigned char subindex,
  unsigned char* data, ssize_t num) {
  can_message_t message;
  memset(&message, 0, sizeof(can_message_t));

  message.id = EPOS_DEVICE_SEND_ID+dev->node_id;
  message.content[0] = EPOS_DEVICE_READ_SEND;
  message.content[1] = index;
  message.content[2] = index >> 8;
  message.content[3] = subindex;

  if (!epos_device_send_message(dev, &message) &&
    !epos_device_receive_message(dev, &message)) {
    memcpy(data, &message.content[4], num);
    ++dev->num_read;
    return EPOS_DEVICE_ERROR_NONE;
  }
  else
    return EPOS_DEVICE_ERROR_READ;
}

int epos_device_write(epos_device_p dev, short index, unsigned char subindex,
  unsigned char* data, ssize_t num) {
  can_message_t message;

  message.id = EPOS_DEVICE_SEND_ID+dev->node_id;
  switch (num) {
    case 1 : message.content[0] = EPOS_DEVICE_WRITE_SEND_1_BYTE;
             break; 
    case 2 : message.content[0] = EPOS_DEVICE_WRITE_SEND_2_BYTE;
             break; 
    case 4 : message.content[0] = EPOS_DEVICE_WRITE_SEND_4_BYTE;
             break; 
    default: return EPOS_DEVICE_ERROR_INVALID_SIZE;
  }
  message.content[1] = index;
  message.content[2] = index >> 8;
  message.content[3] = subindex;
  message.content[4] = data[0];
  message.content[5] = (num > 1) ? data[1] : 0x00;
  message.content[6] = (num > 2) ? data[2] : 0x00;
  message.content[7] = (num > 2) ? data[3] : 0x00;

  if (!epos_device_send_message(dev, &message) &&
    !epos_device_receive_message(dev, &message)) {
    ++dev->num_written;
    return EPOS_DEVICE_ERROR_NONE;
  }
  else
    return EPOS_DEVICE_ERROR_WRITE;
}

int epos_device_store_parameters(epos_device_p dev) {
  return epos_device_write(dev, EPOS_DEVICE_INDEX_STORE,
    EPOS_DEVICE_SUBINDEX_STORE, "save", 4);
}

int epos_device_restore_parameters(epos_device_p dev) {
  return epos_device_write(dev, EPOS_DEVICE_INDEX_STORE,
    EPOS_DEVICE_SUBINDEX_STORE, "load", 4);
}

int epos_device_get_id(epos_device_p dev) {
  unsigned char id;
  epos_device_read(dev, EPOS_DEVICE_INDEX_ID, 0, &id, 1);

  return id;
}

int epos_device_get_can_bitrate(epos_device_p dev) {
  short can_bitrate;
  epos_device_read(dev, EPOS_DEVICE_INDEX_CAN_BITRATE, 0,
    (unsigned char*)&can_bitrate, sizeof(short));

  return epos_device_can_bitrates[can_bitrate];
}

int epos_device_set_can_bitrate(epos_device_p dev, int bitrate) {
  short b;
  for (b = 0; b < sizeof(epos_device_can_bitrates)/sizeof(int); ++b)
    if (epos_device_can_bitrates[b] == bitrate) {
    return epos_device_write(dev, EPOS_DEVICE_INDEX_CAN_BITRATE, 0,
      (unsigned char*)&b, sizeof(short));
  }
  return EPOS_DEVICE_ERROR_INVALID_BITRATE;
}

int epos_device_get_rs232_baudrate(epos_device_p dev) {
  short rs232_baudrate;
  epos_device_read(dev, EPOS_DEVICE_INDEX_RS232_BAUDRATE, 0,
    (unsigned char*)&rs232_baudrate, sizeof(short));

  return epos_device_rs232_baudrates[rs232_baudrate];
}

int epos_device_set_rs232_baudrate(epos_device_p dev, int baudrate) {
  short b;
  for (b = 0; b < sizeof(epos_device_rs232_baudrates)/sizeof(int); ++b)
    if (epos_device_rs232_baudrates[b] == baudrate) {
    return epos_device_write(dev, EPOS_DEVICE_INDEX_RS232_BAUDRATE, 0,
      (unsigned char*)&b, sizeof(short));
  }
  return EPOS_DEVICE_ERROR_INVALID_BAURATE;
}

short epos_device_get_hardware_version(epos_device_p dev) {
  short hardware_version;
  epos_device_read(dev, EPOS_DEVICE_INDEX_VERSION,
    EPOS_DEVICE_SUBINDEX_HARDWARE_VERSION,
    (unsigned char*)&hardware_version, sizeof(short));

  return hardware_version;
}

short epos_device_get_software_version(epos_device_p dev) {
  short software_version;
  epos_device_read(dev, EPOS_DEVICE_INDEX_VERSION,
    EPOS_DEVICE_SUBINDEX_SOFTWARE_VERSION,
    (unsigned char*)&software_version, sizeof(short));

  return software_version;
}

short epos_device_get_status(epos_device_p dev) {
  short status;
  epos_device_read(dev, EPOS_DEVICE_INDEX_STATUS, 0,
    (unsigned char*)&status, sizeof(short));

  return status;
}

short epos_device_get_control(epos_device_p dev) {
  short control;
  epos_device_read(dev, EPOS_DEVICE_INDEX_CONTROL, 0,
    (unsigned char*)&control, sizeof(short));

  return control;
}

int epos_device_set_control(epos_device_p dev, short control) {
  return epos_device_write(dev, EPOS_DEVICE_INDEX_CONTROL, 0,
    (unsigned char*)&control, sizeof(short));
}

int epos_device_reset(epos_device_p dev) {
  int result = epos_device_set_control(dev, EPOS_DEVICE_CONTROL_FAULT_RESET);

  if (!result)
    result = epos_device_set_control(dev, EPOS_DEVICE_CONTROL_SHUTDOWN);

  return result;
}
