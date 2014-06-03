#ifndef PROTOCOL_H
#define PROTOCOL_H

#define MAGIC1 (uint8_t)0xAF
#define MAGIC2 (uint8_t)0xEB

/* First 3 bits */
#define MASK_CMD (uint8_t)0x80
#define MASK_REPLY_OK (uint8_t)0x40
#define MASK_REPLY_ERR (uint8_t)0x20

/* Last 5 bits, up to 0x1F */
#define CMD_ERASE (uint8_t)0x01
#define CMD_READ (uint8_t)0x02
#define CMD_WRITE (uint8_t)0x03
#define CMD_RESET (uint8_t)0x04
#define CMD_GET_FLAGS (uint8_t)0x05
#define CMD_WAKE (uint8_t)0x06
#define CMD_BOOT (uint8_t)0x07
#define CMD_GET_MODE (uint8_t)0x08
#define CMD_GET_VERSION (uint8_t)0x09
#define CMD_GET_SIZE (uint8_t)0x0A

#define FLAG_OK (uint8_t)0x01
#define FLAG_IWDRST (uint8_t)0x02
#define FLAG_SFTRST (uint8_t)0x04

#define MODE_BL (uint8_t)0x01
#define MODE_APP (uint8_t)0x02

#define ERASE_OK (uint8_t)0x10

#define DATA_BUF_SIZE 256

typedef struct {
  uint8_t magic1;
  uint8_t magic2;
  uint8_t type;
  uint8_t len;
} cmd_header_t;


#endif