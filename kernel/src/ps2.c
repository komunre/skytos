#include "ps2.h"
#include "port/port.h"

short ps2_data = 0x60;
short ps2_cmd = 0x64;
short ps2_status = 0x64;

void send_cmd(char cmd, char data)  {
    write_byte_to_port((short)ps2_data, data);
    write_byte_to_port((short)ps2_cmd, cmd);
}

char send_cmd_receive(char cmd, char data) {
    send_cmd(cmd, data);
    return get_resp(ps2_data);
}


char get_resp(short data_port) {
    return read_byte_from_port(data_port);
}