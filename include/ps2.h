#ifndef PS2_H
#define PS2_H

extern short ps2_data;
extern short ps2_cmd;
extern short ps2_status;

void send_cmd(char cmd, char data);

char send_cmd_receive(char cmd, char data);

char get_resp(short data_port);

#endif